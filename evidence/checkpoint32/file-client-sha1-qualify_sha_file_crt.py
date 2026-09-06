from pathlib import Path
import sys,json,struct,hashlib
sys.path.insert(0,'tools');from binary import PE,u32
from verify_checkpoint2 import map_symbols
from capstone import Cs,CS_ARCH_X86,CS_MODE_32,CS_OP_IMM
run='cp32-file-client-sha1-linked1';d=Path('build')/run;p=PE(Path('private/samp.dll'));q=PE(d/'closure.dll');maps=map_symbols(d/'closure.map');md=Cs(CS_ARCH_X86,CS_MODE_32);md.detail=True
pin=json.loads(Path('config/checkpoint31/sdk.json').read_text());link=json.loads((d/'link.json').read_text(encoding='utf-8-sig'));assert pin['libcmt.lib']==link['sdk']['libcmt.lib']
methods=[('_fseek',0xc877f,73,'fseek.obj'),('_ftell',0xc86af,65,'ftell.obj'),('_div',0xc8e73,26,'div.obj'),('_fwrite',0xc7a72,76,'fwrite.obj'),('__SEH_prolog',0xce430,59,'sehprolg.obj'),('__SEH_epilog',0xce46b,17,'sehprolg.obj')]
externals={0xcff1d:('__lock_file','_file.obj'),0xcff6f:('__unlock_file','_file.obj'),0xc86f0:('__fseek_lk','fseek.obj'),0xc854d:('__ftell_lk','ftell.obj'),0xc796b:('__fwrite_lk','fwrite.obj'),0xc74c4:('__except_handler3','exsup3.obj')}
def addr(n,owner):
 hits=maps[n];assert len(hits)==1 and hits[0][1].lower()==('libcmt:'+owner).lower();return hits[0][0]
mapping={r:addr(n,o) for n,r,size,o in methods};mapping.update({r:addr(n,o) for r,(n,o) in externals.items()})
for n,r,size,o in methods:
 for off in range(size):mapping[r+off]=mapping[r]+off
scopes=[]
for table,parent in [(0xf03e0,0xc877f),(0xf03d0,0xc86af),(0xf0350,0xc7a72)]:
 actual=u32(q.read(mapping[parent]-q.base+3,4),0);mapping[table]=actual;raw=bytearray(p.read(table,16));bindings=[]
 for off in range(0,16,4):
  if table+off in p.relocations:
   old=u32(raw,off)-p.base;assert old in mapping and actual-q.base+off in q.relocations;struct.pack_into('<I',raw,off,mapping[old]);bindings.append(dict(offset=off,original_target_rva=old,linked_target_va=mapping[old]))
 assert {a-table for a in p.relocations if table<=a<table+16}=={a-(actual-q.base) for a in q.relocations if actual-q.base<=a<actual-q.base+16}
 assert bytes(raw)==q.read(actual-q.base,16)
 scopes.append(dict(original_rva=table,linked_rva=actual-q.base,size=16,scope_record_size=12,trailing_zero_alignment_size=4,whole_bytes_equal=True,all_PE_relocations_equal=True,bindings=bindings))
code=[]
for name,r,size,owner in methods:
 original=p.read(r,size);raw=bytearray(original);va=mapping[r];bindings=[]
 rel={x-r for x in p.relocations if r<=x<r+size};assert rel=={x-(va-q.base) for x in q.relocations if va-q.base<=x<va-q.base+size}
 for off in rel:
  target=u32(raw,off)-p.base;assert target in mapping;struct.pack_into('<I',raw,off,mapping[target]);bindings.append(dict(offset=off,kind='HIGHLOW',original_target_rva=target,linked_target_va=mapping[target]))
 for ins in md.disasm(original,p.base+r):
  if not (ins.mnemonic.startswith('j') or ins.mnemonic=='call') or not ins.operands or ins.operands[0].type!=CS_OP_IMM:continue
  target=ins.operands[0].imm-p.base
  if r<=target<r+size:continue
  assert target in mapping and ins.imm_size==4,(name,hex(target));off=ins.address-p.base-r+ins.imm_offset;struct.pack_into('<I',raw,off,(mapping[target]-(va+ins.address-p.base-r+ins.size))&0xffffffff);bindings.append(dict(offset=off,kind='REL32',original_target_rva=target,linked_target_va=mapping[target]))
 assert bytes(raw)==q.read(va-q.base,size),name
 code.append(dict(symbol=name,original_rva=r,linked_rva=va-q.base,size=size,SDK_member='LIBCMT:'+owner,whole_resolved_equal=True,all_PE_relocations_equal=True,bindings=bindings))
x=dict(status='PINNED_CRT_FSEEK_FTELL_DIV_FWRITE_WHOLE_WRAPPERS_AND_SCOPES_QUALIFIED',run=run,linked_sha256=hashlib.sha256(q.data).hexdigest(),map_sha256=hashlib.sha256((d/'closure.map').read_bytes()).hexdigest(),libcmt_sha256=pin['libcmt.lib'],new_source_credit=0,code=code,SEH_scopes=scopes,real_external_boundaries=[dict(symbol=n,original_rva=r,linked_rva=mapping[r]-q.base,SDK_member='LIBCMT:'+o,source_credit=0) for r,(n,o) in externals.items()])
Path('build/agent-independent/sha-file-crt-qualified.json').write_text(json.dumps(x,indent=2)+'\n');print('PASS',len(code),'whole methods +3 whole scopes, zero credit')
