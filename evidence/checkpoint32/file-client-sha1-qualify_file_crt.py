from pathlib import Path
import sys,json,struct,hashlib
sys.path.insert(0,'tools')
from binary import PE,u32
from verify_checkpoint2 import map_symbols,import_slots
from capstone import Cs,CS_ARCH_X86,CS_MODE_32,CS_OP_IMM
run='cp32-file-client-sha1-linked1';d=Path('build')/run;p=PE(Path('private/samp.dll'));q=PE(d/'closure.dll');maps=map_symbols(d/'closure.map');pi=import_slots(p);qi=import_slots(q);md=Cs(CS_ARCH_X86,CS_MODE_32);md.detail=True
link=json.loads((d/'link.json').read_text(encoding='utf-8-sig'));pin=json.loads(Path('config/checkpoint31/sdk.json').read_text());assert link['sdk']['libcmt.lib']==pin['libcmt.lib'];assert link['sdk']['kernel32.lib']==pin['kernel32.lib']
methods=[('__findfirst',0xc81e4,221,'findfile.obj'),('__findnext',0xc82c1,216,'findfile.obj'),('__findclose',0xc8160,32,'findfile.obj'),('__mkdir',0xc87c8,44,'mkdir.obj'),('__unlink',0xc815b,5,'unlink.obj'),('__strnicmp',0xc84ce,127,'strnicmp.obj'),('__stricmp',0xc7f72,105,'stricmp.obj'),('___timet_from_ft',0xc8180,100,'findfile.obj'),('_remove',0xc8131,42,'unlink.obj'),('___ascii_strnicmp',0xd1490,97,'_strnicm.obj'),('___ascii_stricmp',0xd0e90,78,'_stricmp.obj'),('__errno',0xd096f,9,'dosmap.obj'),('@__security_check_cookie@4',0xd1119,14,'secchk.obj')]
external={0xd0981:('__dosmaperr','dosmap.obj'),0xcce0c:('__getptd','tidtable.obj'),0xcec82:('___updatetlocinfo','setlocal.obj'),0xc915e:('___tolower_mt','tolower.obj'),0xd0ede:('___loctotime_t','dtoxtime.obj'),0xd10e8:('_report_failure','secchk.obj'),0xd0ff0:('_strcpy','strcat.obj')}
static={0x1176b4:('___security_cookie','seccook.obj'),0x117864:('___ptlocinfo','setlocal.obj')}
def mapaddr(n,member):
 hits=maps[n];assert len(hits)==1 and hits[0][1].lower()==('libcmt:'+member).lower(),(n,hits,member)
 return hits[0][0]
mapping={r:mapaddr(n,owner) for n,r,size,owner in methods};mapping.update({r:mapaddr(n,owner) for r,(n,owner) in external.items()});mapping.update({r:mapaddr(n,owner) for r,(n,owner) in static.items()})
inv={f['rva']:f for f in json.loads(Path('config/checkpoint31/inventory.json').read_text())['functions']};code=[]
for name,r,size,owner in methods:
 assert inv[r]['chunks']==[dict(rva=r,size=size,sha256=hashlib.sha256(p.read(r,size)).hexdigest())]
 va=mapping[r];raw=bytearray(p.read(r,size));bindings=[]
 # Absolute references: each original+linked HIGHLOW set must agree exactly.
 offsets={a-r for a in p.relocations if r<=a<r+size};assert offsets=={a-(va-q.base) for a in q.relocations if va-q.base<=a<va-q.base+size}
 for off in sorted(offsets):
  old=u32(raw,off);actual=u32(q.read(va-q.base+off,4),0)
  if old in pi:
   assert qi.get(actual)==pi[old];target=actual;identity=dict(import_dll=pi[old][0],import_name=pi[old][1])
  else:
   assert old-p.base in mapping,(name,hex(old));target=mapping[old-p.base];assert target==actual;identity=dict(symbol=static[old-p.base][0] if old-p.base in static else next(x[0] for x in methods if x[1]==old-p.base))
  struct.pack_into('<I',raw,off,target);bindings.append(dict(offset=off,kind='HIGHLOW',original_target_va=old,linked_target_va=target,identity=identity))
 # Direct relative branches/calls: preserve intra-function offsets; resolve only actual named CRT providers.
 for ins in md.disasm(p.read(r,size),p.base+r):
  if not (ins.mnemonic.startswith('j') or ins.mnemonic=='call') or not ins.operands or ins.operands[0].type!=CS_OP_IMM:continue
  target=ins.operands[0].imm
  if p.base+r<=target<p.base+r+size:continue
  assert ins.imm_size==4 and target-p.base in mapping,(name,hex(target));dest=mapping[target-p.base];off=ins.address-p.base-r+ins.imm_offset;struct.pack_into('<I',raw,off,(dest-(va+ins.address-p.base-r+ins.size))&0xffffffff)
  nm=external[target-p.base][0] if target-p.base in external else next(x[0] for x in methods if x[1]==target-p.base)
  bindings.append(dict(offset=off,kind='REL32',original_target_rva=target-p.base,linked_target_va=dest,identity=dict(symbol=nm)))
 assert bytes(raw)==q.read(va-q.base,size),(name,hex(r),hex(va))
 code.append(dict(symbol=name,SDK_member='LIBCMT:'+owner,original_rva=r,linked_rva=va-q.base,size=size,whole_resolved_equal=True,all_PE_relocations_equal=True,bindings=bindings))
a=dict(status='PINNED_CRT_SEVEN_PROVIDERS_AND_SIX_HELPERS_FULL_BYTES_QUALIFIED',run=run,linked_sha256=hashlib.sha256((d/'closure.dll').read_bytes()).hexdigest(),map_sha256=hashlib.sha256((d/'closure.map').read_bytes()).hexdigest(),sdk_pins={k:pin[k] for k in ['libcmt.lib','kernel32.lib']},new_source_credit=0,code=code,named_external_boundaries=[dict(symbol=n,SDK_member='LIBCMT:'+owner,original_rva=r,linked_rva=mapping[r]-q.base,body_credit=0) for r,(n,owner) in external.items()],named_CRT_state=[dict(symbol=n,SDK_member='LIBCMT:'+owner,original_rva=r,linked_rva=mapping[r]-q.base,body_credit=0) for r,(n,owner) in static.items()],scope='Seven required APIs + six bounded stable helpers fully compared, no address masking. Imports matched by actual DLL+name, every absolute PE relocation required on both images. Remaining named CRT helper/state boundaries carry zero implementation credit.')
Path('build/agent-independent/file-crt-seven-qualified.json').write_text(json.dumps(a,indent=2)+'\n');print('PASS',len(code),'complete CRT functions',sum(x['size'] for x in code),'bytes, zero credit')
