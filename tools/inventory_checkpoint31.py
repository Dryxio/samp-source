"""Export frozen IDA hints and independently read PE imports, ranges and edges.

IDA names and function boundaries are leads, never accepted matching coverage.
No original bytes or disassembly are written to the versioned inventory.
"""
import json
import hashlib
from collections import Counter
from pathlib import Path
import idb
import idb.analysis
from capstone import Cs,CS_ARCH_X86,CS_MODE_32
from capstone.x86 import X86_OP_IMM,X86_OP_MEM
from binary import PE,need,sha,u32
from verify import ROOT
from verify_checkpoint2 import import_slots


def classify(name):
    if any(x in name for x in ('RakNet','RakPeer','RakClient','RakServer','ReliabilityLayer','SocketLayer','RPCMap','CheckSum','Huffman','StringCompressor','StringTable','DataBlockEncryptor','PlayerID','NetworkID','InternalPacketPool')):
        return 'network'
    if any(x in name for x in ('DXUT','D3DX','Direct3D','ID3DXFontHook')): return 'interface'
    if any(x in name.lower() for x in ('md5','sha1','rijndael','checksum')): return 'utilities'
    if name.startswith(('__','_')): return 'runtime-or-c-symbol'
    if name.startswith('?'): return 'client-or-unclassified-cpp'
    return 'unknown'


def main():
    dest=ROOT/'config/checkpoint31'
    reference=PE(ROOT/'private/samp.dll',json.loads((ROOT/'config/manifest.json').read_text())['target_sha256'])
    database=ROOT/'private/checkpoint31/samp.idb'
    md=Cs(CS_ARCH_X86,CS_MODE_32);md.detail=True
    functions=[];symbols={};edges=[];decode_failures=[]
    with idb.from_file(str(database)) as db:
        api=idb.IDAPython(db)
        need(idb.analysis.Root(db).md5.lower()==hashlib.md5(reference.data).hexdigest(),'IDB input identity differs')
        for ea,name in api.idautils.Names():
            if reference.base<=ea<reference.base+reference.image_size:
                symbols[name]=ea-reference.base
        allfunc=idb.analysis.Functions(db).functions
        for ea,func in sorted(allfunc.items()):
            if func.flags&func.FUNC_TAIL: continue
            name=idb.analysis.Function(db,ea).get_name()
            chunks=[(ea,func.endEA-ea)]
            try: chunks.extend((c.effective_address,c.length) for c in idb.analysis.Function(db,ea).get_chunks())
            except KeyError: pass
            record=dict(rva=ea-reference.base,name=name,classification=classify(name),
                        status='IDENTIFIED_ONLY',chunks=[])
            symbols.setdefault(name,ea-reference.base)
            for start,size in chunks:
                raw=reference.read(start-reference.base,size)
                flags=[db.id1.get_flags(address) for address in range(start,start+size)]
                need(bytes(f&255 for f in flags)==raw,'IDB bytes differ from reference at '+hex(start))
                record['chunks'].append(dict(rva=start-reference.base,size=size,sha256=sha(raw)))
                for off,flag in enumerate(flags):
                    if not api.ida_bytes.is_code(flag): continue
                    ins=next(md.disasm(raw[off:off+15],start+off,count=1),None)
                    if not ins:
                        decode_failures.append(start+off-reference.base);continue
                    if ins.mnemonic not in ('call','jmp'):continue
                    operand=ins.operands[0]
                    edge=dict(owner=ea-reference.base,site=ins.address-reference.base,kind=ins.mnemonic)
                    if operand.type==X86_OP_IMM:
                        edge.update(target=operand.imm-reference.base,target_kind='direct')
                    elif operand.type==X86_OP_MEM and not operand.mem.base and not operand.mem.index:
                        edge.update(target=operand.mem.disp-reference.base,target_kind='absolute-slot')
                    else: edge.update(target_kind='indirect-unresolved')
                    edges.append(edge)
            functions.append(record)
    output=dict(reference_sha256=sha(reference.data),idb_sha256=sha(database.read_bytes()),
                provenance='Pinned upstream IDB; every function chunk checked against reference bytes; labels and boundaries remain hints',
                functions=functions,symbols=symbols,edges=edges,decode_failures=decode_failures)
    (dest/'inventory.json').write_text(json.dumps(output,indent=2)+'\n')
    pe_header=u32(reference.data,0x3c)+24
    imports=[dict(slot_rva=va-reference.base,dll=dll,name=name) for va,(dll,name) in sorted(import_slots(reference).items())]
    summary=dict(reference_sha256=sha(reference.data),image_base=reference.base,image_size=reference.image_size,
                 entry_rva=u32(reference.data,pe_header+16),
                 sections=[{k:s[k] for k in ('name','rva','virtual_size','size','flags')} for s in reference.sections],
                 imports=imports,relocation_count=len(reference.relocations),functions_identified=len(functions),
                 classification_counts=dict(Counter(f['classification'] for f in functions)),
                 indirect_edges=sum(e['target_kind']=='indirect-unresolved' for e in edges),
                 status='MAP_ONLY_NOT_MATCHING_COVERAGE')
    (dest/'pe.json').write_text(json.dumps(summary,indent=2)+'\n')
    print('PASS inventory:',len(functions),'function hints;',len(imports),'imports;',len(edges),'call/jump sites')


if __name__=='__main__': main()
