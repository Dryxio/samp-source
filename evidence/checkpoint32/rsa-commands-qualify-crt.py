import json,sys,struct
from pathlib import Path
sys.path.insert(0,'tools');from binary import PE,sha
from verify_checkpoint2 import map_symbols,import_slots
from capstone import Cs,CS_ARCH_X86,CS_MODE_32,CS_OP_IMM
run='cp32-rsa-commands-linked1';d=Path('build')/run;p=PE('private/samp.dll');q=PE(d/'closure.dll');maps=map_symbols(d/'closure.map');pin=json.load(open('config/checkpoint31/sdk.json'))['libcmt.lib'];assert json.load(open(d/'link.json',encoding='utf-8-sig'))['sdk']['libcmt.lib']==pin
methods=[('_sscanf',0xca38b,52,'sscanf.obj'),('_mblen',0xca3bf,137,'mblen.obj')]
external={0xcbdf0:('_strlen','strlen.obj'),0xd2a90:('__input','input.obj'),0xcce0c:('__getptd','tidtable.obj'),0xcec82:('___updatetlocinfo','setlocal.obj'),0x117864:('___ptlocinfo','setlocal.obj'),0x26ef8c:('___lc_codepage','nlsdata2.obj')}
def addr(n,owner):
 hits=maps[n];assert len(hits)==1 and hits[0][1].lower()=='libcmt:'+owner;return hits[0][0]
mapping={r:addr(n,owner) for n,r,z,owner in methods};mapping.update({r:addr(n,owner) for r,(n,owner) in external.items()})
identity=('kernel32.dll','MultiByteToWideChar');assert import_slots(p)[p.base+0xe5200]==identity;hits=[a for a,v in import_slots(q).items() if v==identity];assert len(hits)==1;mapping[0xe5200]=hits[0]
md=Cs(CS_ARCH_X86,CS_MODE_32);md.detail=True;rows=[]
for n,r,z,owner in methods:
 raw=bytearray(p.read(r,z));va=mapping[r];bindings=[]
 oldrel={a-r for a in p.relocations if r<=a<r+z};assert oldrel=={a-(va-q.base) for a in q.relocations if va-q.base<=a<va-q.base+z}
 for off in oldrel:
  target=struct.unpack_from('<I',raw,off)[0]-p.base;assert target in mapping;struct.pack_into('<I',raw,off,mapping[target]);bindings.append(dict(offset=off,kind='HIGHLOW',original_target_rva=target,linked_va=mapping[target]))
 for ins in md.disasm(p.read(r,z),p.base+r):
  if not (ins.mnemonic.startswith('j') or ins.mnemonic=='call') or not ins.operands or ins.operands[0].type!=CS_OP_IMM:continue
  target=ins.operands[0].imm-p.base
  if r<=target<r+z:continue
  assert target in mapping and ins.imm_size==4;off=ins.address-p.base-r+ins.imm_offset;struct.pack_into('<I',raw,off,(mapping[target]-(va+ins.address-p.base-r+ins.size))&0xffffffff);bindings.append(dict(offset=off,kind='REL32',original_target_rva=target,linked_va=mapping[target]))
 assert bytes(raw)==q.read(va-q.base,z);rows.append(dict(symbol=n,original_rva=r,size=z,linked_va=va,owner='LIBCMT:'+owner,whole_bytes_equal=True,all_PE_relocations_equal=True,bindings=bindings))
out=dict(status='PINNED_COMPLETE_SSCANF52_MBLEN137_QUALIFIED',run=run,linked_sha256=sha(q.data),library_sha256=pin,regions=rows,boundaries=[dict(symbol=n,original_rva=r,linked_va=mapping[r],owner='LIBCMT:'+owner,kind='runtime-data' if r>0x100000 else 'runtime-code') for r,(n,owner) in external.items()],import_identity=identity,new_credit=0)
Path('build/rsa-commands-crt-qualified.json').write_text(json.dumps(out,indent=2)+'\n');print('PASS scanf52/mblen137 whole source CRT wrappers and true boundaries')
