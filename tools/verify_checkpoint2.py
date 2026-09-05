"""Verify complete COFF regions, actual MS LINK output, EH data and graph bindings.

No byte masks. Each fixup is replayed twice: once at the declared R5 layout and
once at the linker's actual layout. Both resulting complete regions must match.
"""
import argparse
import json
import re
import struct
from pathlib import Path

from capstone.x86 import X86_OP_IMM, X86_OP_MEM, X86_REG_FS
from capstone import CS_GRP_JUMP
from binary import COFF, PE, need, sha, u16, u32
from verify import ROOT, decode


def map_symbols(path):
    symbols={}
    for line in path.read_text().splitlines():
        m=re.match(r'\s+[0-9a-fA-F]{4}:[0-9a-fA-F]{8}\s+(\S+)\s+([0-9a-fA-F]{8})\s+(.*)',line)
        if m:
            name,address,tail=m.groups()
            symbols.setdefault(name,[]).append((int(address,16),tail.split()[-1]))
    return symbols


def symbol_address(symbols,name,owner=None):
    hits={address for address,source in symbols.get(name,[]) if owner is None or source==owner+'.obj'}
    need(len(hits)==1,'missing/ambiguous MAP symbol: '+name)
    return hits.pop()


def relocs(obj,section_index):
    sec=obj.sections[section_index-1]
    result=[];used=set()
    for i in range(sec['reloc_count']):
        off,idx,kind=struct.unpack_from('<IIH',obj.data,sec['reloc_ptr']+i*10)
        need(kind in (6,20),'unsupported COFF fixup')
        need(off+4<=sec['size'] and not used.intersection(range(off,off+4)),'bad/overlapping fixup')
        used.update(range(off,off+4))
        need(idx in obj.symbols,'bad relocation symbol index')
        result.append(dict(offset=off,kind=kind,symbol=obj.symbols[idx]))
    return result


def import_slots(pe):
    d=pe.data;p=u32(d,0x3c)+24
    rva,size=u32(d,p+104),u32(d,p+108)
    slots={}
    def cstr(r):
        out=bytearray()
        while pe.read(r,1)!=b'\0': out.extend(pe.read(r,1));r+=1
        return out.decode('ascii')
    for off in range(0,size,20):
        desc=pe.read(rva+off,20)
        original,_,_,name,iat=struct.unpack('<IIIII',desc)
        if not name: break
        dll=cstr(name)
        n=0
        while True:
            thunk=u32(pe.read((original or iat)+n*4,4),0)
            if not thunk: break
            imp=('#'+str(thunk&65535)) if thunk&0x80000000 else cstr(thunk+2)
            slots[pe.base+iat+n*4]=(dll.lower(),imp)
            n+=1
    return slots


class Checkpoint:
    def __init__(self,run,check_fresh=True):
        self.directory=ROOT/'build'/run
        self.manifest=json.loads((ROOT/'config/checkpoint2/regions.json').read_text())
        self.reference=PE(ROOT/'private/samp.dll',self.manifest['target_sha256'])
        self.linked=PE(self.directory/'capsule.dll')
        self.maps=map_symbols(self.directory/'capsule.map')
        self.build=json.loads((self.directory/'build.json').read_text(encoding='utf-8-sig'))
        if check_fresh:
            pin=json.loads((ROOT/'config/toolchain.json').read_text())
            need(self.build['toolchain']==pin['tools'],'changed compiler or linker')
            need(self.build['options']==pin['options'],'changed compile options')
            for file,digest in self.build['sources'].items():
                need(sha((ROOT/file).read_bytes())==digest,'stale source '+file)
                need(sha((self.directory/file).read_bytes())==digest,'altered source snapshot '+file)
            need(sha(self.linked.data)==self.build['dll_sha256'],'altered DLL')
            need(sha((self.directory/'capsule.map').read_bytes())==self.build['map_sha256'],'altered MAP')
            need(self.build['native_exit']==0,'native tests did not pass')
            need(sha((self.directory/'native.exe').read_bytes())==self.build['native_sha256'],'altered native executable')
            need(sha((ROOT/'tests/checkpoint2/native.cpp').read_bytes())==self.build['native_source_sha256'],'stale native test source')
            need(sha((self.directory/'native.cpp').read_bytes())==self.build['native_source_sha256'],'altered native source snapshot')
            need(sha((self.directory/'native.log').read_bytes())==self.build['native_log_sha256'],'altered native test log')
            need(sha((ROOT/'config/checkpoint2/exports.def').read_bytes())==self.build['exports_sha256'],'changed exports')
            libraries=json.loads((ROOT/'config/checkpoint2/libraries.json').read_text())
            need(self.build['libraries']==libraries,'changed vendor CRT libraries')
        self.objects={}
        for record in self.build['units']:
            path=self.directory/record['object']
            need(sha(path.read_bytes())==record['object_sha256'],'altered object')
            self.objects[path.stem]=COFF(path)
        self.regions=[];self.by_section={}
        for contract in self.manifest['regions']:
            region=dict(contract)
            obj=self.objects[region['unit']]
            anchors=[s for s in obj.names.get(region['anchor'],[]) if s['section']>0]
            need(len(anchors)==1,'ambiguous region anchor')
            anchor=anchors[0]
            need(anchor['value']==region['anchor_offset'],'anchor offset changed')
            sec=obj.sections[anchor['section']-1]
            need(sec['size']==region['size'],'full region size mismatch '+region['anchor'])
            need(sha(self.reference.read(region['rva'],region['size']))==region['sha256'],'bad original region contract')
            region.update(section=anchor['section'],raw=sec['bytes'],relocs=relocs(obj,anchor['section']))
            key=(region['unit'],region['section'])
            need(key not in self.by_section,'duplicate section contract')
            self.by_section[key]=region;self.regions.append(region)
            if region['anchor'] in self.maps:
                region['linked_va']=symbol_address(self.maps,region['anchor'],region['unit'])-anchor['value']
        self.bind_data_sections()

    def bind_data_sections(self):
        # Public and static code anchors come from the MAP. Non-public EH data
        # is located by following identified COFF fixups in these linked regions.
        progress=True
        while progress:
            progress=False
            for region in self.regions:
                if 'linked_va' not in region: continue
                code=self.linked.read(region['linked_va']-self.linked.base,region['size'])
                for rel in region['relocs']:
                    s=rel['symbol'];off=rel['offset']
                    if s['section']<=0: continue
                    key=(region['unit'],s['section'])
                    need(key in self.by_section,'uncontracted defined dependency '+s['name'])
                    target=self.by_section[key]
                    resolved=u32(code,off)
                    if rel['kind']==20: resolved=(resolved+region['linked_va']+off+4)&0xffffffff
                    found=(resolved-u32(region['raw'],off)-s['value'])&0xffffffff
                    if 'linked_va' in target:
                        need(target['linked_va']==found,'conflicting linkage of '+s['name'])
                    else:
                        target['linked_va']=found;progress=True
        occupied=set();original=set()
        for r in self.regions:
            need('linked_va' in r,'unresolved linked data region')
            actual=set(range(r['linked_va'],r['linked_va']+r['size']))
            expected=set(range(r['rva'],r['rva']+r['size']))
            need(not occupied.intersection(actual) and not original.intersection(expected),'overlapping region identity')
            occupied.update(actual);original.update(expected)

    def target(self,region,symbol,linked):
        if symbol['section']>0:
            dep=self.by_section[(region['unit'],symbol['section'])]
            return (dep['linked_va'] if linked else self.reference.base+dep['rva'])+symbol['value'],False
        name=symbol['name']
        # Cross-object source function definitions are resolved to a covered region.
        candidates=[]
        for other in self.regions:
            for s in self.objects[other['unit']].names.get(name,[]):
                if s['section']==other['section'] and s['storage']==2:
                    candidates.append((other,s))
        need(len(candidates)<=1,'ambiguous cross-object definition')
        if candidates:
            dep,s=candidates[0]
            value=(dep['linked_va'] if linked else self.reference.base+dep['rva'])+s['value']
            if linked: need(value==symbol_address(self.maps,name),'cross-object linker target differs')
            return value,False
        external=self.manifest['externals'].get(name)
        need(external is not None,'unresolved external '+name)
        absolute=external['kind']=='absolute-fs'
        value=external['address'] if absolute else self.reference.base+external['rva']
        if linked:
            actual=symbol_address(self.maps,name)
            if absolute: need(actual==value,'FS pseudo-symbol is not absolute zero')
            value=actual
        return value,absolute

    def replay(self,region,linked):
        result=bytearray(region['raw']);abs_sites=set()
        base=region['linked_va'] if linked else self.reference.base+region['rva']
        instructions=decode(region['raw'][:region['code_size']],base) if region['code_size'] else []
        for rel in region['relocs']:
            off=rel['offset'];kind=rel['kind']
            target,absolute=self.target(region,rel['symbol'],linked)
            if off<region['code_size']:
                ins=next((i for i in instructions if i.address-base<=off<i.address-base+i.size),None)
                need(ins is not None,'fixup outside instruction')
                start=ins.address-base
                if kind==20:
                    need(ins.mnemonic in ('call','jmp') and ins.imm_size==4 and off==start+ins.imm_offset,'REL32 not call/tail-call')
                else:
                    need((ins.disp_size==4 and off==start+ins.disp_offset) or
                         (ins.imm_size==4 and off==start+ins.imm_offset),'DIR32 not address operand')
                if absolute:
                    need(kind==6 and any(o.type==X86_OP_MEM and o.mem.segment==X86_REG_FS for o in ins.operands),
                         'absolute zero exemption is only for FS addressing')
            else:
                need(kind==6 and off%4==0,'invalid data pointer fixup')
            value=target+u32(region['raw'],off)
            if kind==20: value-=base+off+4
            elif not absolute: abs_sites.add(off)
            struct.pack_into('<I',result,off,value&0xffffffff)
        pe=self.linked if linked else self.reference
        rva=base-pe.base
        actual_sites={x-rva for x in pe.relocations if rva<=x<rva+region['size']}
        need(abs_sites==actual_sites,'PE relocation coverage disagrees with COFF graph')
        return bytes(result)

    def verify(self):
        results=[]
        original_slots=import_slots(self.reference);linked_slots=import_slots(self.linked)
        ext=self.manifest['externals']['__imp__VirtualProtect@16']
        identity=(ext['dll'].lower(),ext['import'])
        need(original_slots[self.reference.base+ext['rva']]==identity,'wrong original IAT identity')
        need(linked_slots[symbol_address(self.maps,'__imp__VirtualProtect@16')]==identity,'wrong linked IAT identity')
        for region in self.regions:
            expected=self.reference.read(region['rva'],region['size'])
            normalized=self.replay(region,False)
            need(normalized==expected,'R5 region mismatch '+region['anchor'])
            actual=self.linked.read(region['linked_va']-self.linked.base,region['size'])
            need(self.replay(region,True)==actual,'actual linked region mismatch '+region['anchor'])
            if region['code_size']:
                code=expected[:region['code_size']]
                instructions=decode(code,self.reference.base+region['rva'])
                starts={i.address for i in instructions}
                for ins in instructions:
                    if ins.group(CS_GRP_JUMP) and ins.operands[0].type==X86_OP_IMM:
                        external=any(r['kind']==20 and r['offset']==ins.address-self.reference.base-region['rva']+ins.imm_offset for r in region['relocs'])
                        need(external or ins.operands[0].imm in starts,'uncontracted jump target')
                if region['anchor']=='_FormatBits':
                    for off in range(60,128,4): need(u32(expected,off) in starts,'switch target is not an instruction boundary')
                    need(len([i for i in instructions if i.mnemonic=='jmp'])==1,'switch dispatch changed')
            results.append(dict(unit=region['unit'],anchor=region['anchor'],rva=region['rva'],
                                linked_rva=region['linked_va']-self.linked.base,size=region['size'],
                                code_bytes=region['code_size'],fixups=len(region['relocs']),sha256=sha(expected),status='LINKED_REGION_EXACT'))
        # SafeSEH object metadata must name the same generated handler we matched.
        for unit in ('buffer','layer'):
            obj=self.objects[unit]
            sx=[s for s in obj.sections if s['name']=='.sxdata']
            need(len(sx)==1 and sx[0]['size']==4,'unexpected SafeSEH metadata')
            sym=obj.symbols[u32(sx[0]['bytes'],0)]
            need(sym['name'].startswith('__ehhandler$??1'),'SafeSEH entry is not the matched destructor handler')
        crt=[]
        for name,rva,size,offset,callee,callee_rva in [
            ('??3@YAXPAX@Z',0xc648a,5,1,'_free',0xc6a3d),
            ('___CxxFrameHandler',0xc6526,54,34,'___InternalCxxFrameHandler',0xccd31)]:
            address=symbol_address(self.maps,name)
            code=bytearray(self.linked.read(address-self.linked.base,size))
            need(code[offset-1] in (0xe8,0xe9),'CRT transfer encoding changed')
            target=(address+offset+4+u32(code,offset))&0xffffffff
            need(target==symbol_address(self.maps,callee),'CRT transfer points to wrong symbol')
            struct.pack_into('<I',code,offset,(callee_rva-rva-offset-4)&0xffffffff)
            need(bytes(code)==self.reference.read(rva,size),'vendor CRT entry mismatch '+name)
            crt.append(dict(symbol=name,rva=rva,size=size,status='VENDOR_CRT_ENTRY_EXACT'))
        return dict(result='PASS',regions=results,crt_entries=crt,
                    code_bytes=sum(r['code_bytes'] for r in results),
                    data_padding_bytes=sum(r['size']-r['code_bytes'] for r in results),
                    scope='source regions and EH/table data exact at R5 layout; actual linked edges verified; not whole DLL identity',
                    native_log=(self.directory/'native.log').read_text(encoding='utf-8-sig'),build=self.build)


def main():
    p=argparse.ArgumentParser();p.add_argument('--run',default='cp2-baseline');a=p.parse_args()
    result=Checkpoint(a.run).verify()
    (ROOT/'build'/a.run/'verification.json').write_text(json.dumps(result,indent=2)+'\n')
    print(f"PASS {a.run}: {len(result['regions'])} complete linked regions, {result['code_bytes']} code + {result['data_padding_bytes']} data/padding bytes; CRT entries exact")


if __name__=='__main__': main()
