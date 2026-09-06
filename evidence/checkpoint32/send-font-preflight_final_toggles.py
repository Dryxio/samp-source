import sys,json,struct
from pathlib import Path
sys.path.insert(0,'tools')
from binary import PE,COFF,sha,u32
from verify_checkpoint2 import relocs
from verify_actor_closure import initial_bytes
from rank_uncovered_candidates import accepted_code
from capstone import Cs,CS_ARCH_X86,CS_MODE_32
p=PE('private/samp.dll');run=Path('build/cp32-final-toggles-trial1');units=['closure_cmd_final_toggles','closure_store_object_lighting_toggle','closure_store_debug_labels_flag'];objs={u:COFF(run/(u+'.obj')) for u in units}
roots={'?cmdCmpStat@@YAXPAD@Z':(0x689f0,1),'?cmdToggleObjectLight@@YAXPAD@Z':(0x69000,19),'?cmdDebugLabels@@YAXPAD@Z':(0x69020,16)}
# Original SetupCommands whole body directly registers all three actual pointers.
ins=list(Cs(CS_ARCH_X86,CS_MODE_32).disasm(p.read(0x69110,593),p.base+0x69110));sites={}
for n,(r,z) in roots.items():
 a=[i.address-p.base for i in ins if i.mnemonic=='push' and i.op_str==hex(p.base+r)];assert len(a)==1,(n,a);sites[n]=a[0]
seeds={units[0]:{n:r for n,(r,z) in roots.items()}};todo=[]
for n,(r,z) in roots.items():
 s=next(v for v in objs[units[0]].names[n] if v['section']>0);assert objs[units[0]].sections[s['section']-1]['size']==z;todo.append((units[0],s['section'],r))
rows=[];seen={};store_expected={'?r5ObjectLightingToggle@@3HA':('closure_store_object_lighting_toggle',0x1166c0,4,1),'?bShowDebugLabels@@3_NA':('closure_store_debug_labels_flag',0x26ebb8,1,0)}
while todo:
 u,si,r=todo.pop();k=(u,si)
 if k in seen:assert seen[k]==r;continue
 seen[k]=r;o=objs[u];sec=o.sections[si-1];original=initial_bytes(p,r,sec['size']);raw=bytearray(sec['bytes'] if sec['bytes'] else bytes(sec['size']));bindings=[]
 for f in relocs(o,si):
  off=f['offset'];s=f['symbol'];a=u32(raw,off);target=u32(original,off)
  assert f['kind']==6
  if s['section']>0:todo.append((u,s['section'],target-a-p.base-s['value']))
  else:
   su,sr,sz,initial=store_expected[s['name']];assert target==p.base+sr+a;sym=next(v for v in objs[su].names[s['name']] if v['section']>0);ss=objs[su].sections[sym['section']-1];assert ss['size']==sz and sym['value']==0;assert initial_bytes(p,sr,sz)==initial.to_bytes(sz,'little');todo.append((su,sym['section'],sr))
  struct.pack_into('<I',raw,off,target);bindings.append(dict(symbol=s['name'],site_rva=r+off,kind=f['kind'],target_va=target))
 assert bytes(raw)==original,(u,hex(r),raw.hex(),original.hex())
 assert {x for x in p.relocations if r<=x<r+sec['size']}=={b['site_rva'] for b in bindings}
 anchor=next(v for v in o.symbols.values() if v['section']==si and v['name']!=sec['name'])
 rows.append(dict(unit=u,section=si,anchor=anchor['name'],rva=r,size=sec['size'],kind='code' if sec['flags']&0x20 else 'data',sha256=sha(original),bindings=bindings))
covered=accepted_code('evidence/checkpoint32/packet-registration-acceptance.json');new={a for r in rows if r['kind']=='code' for a in range(r['rva'],r['rva']+r['size'])}-covered;assert len(new)==36
out=dict(status='PASS_WHOLE_CODE_AND_STORES',run=run.name,baseline=len(covered),new_unique_code=len(new),source025_direct=0,regions=rows,original_registration_sites=sites,identity='Actual SetupCommands pushes each original function. Debug bool real main global; object BOOL initialized1, no stronger inferred state semantics.',owner_transfer='Debug bool main definition must become extern before integrating store; no new allocation.',sources={str(Path('client/saco')/(u+'.cpp')):sha((Path('client/saco')/(u+'.cpp')).read_bytes()) for u in units})
Path('build/final-toggles-reviewed-manifest.json').write_text(json.dumps(out,indent=2)+'\n');Path('build/final-toggles-seeds.json').write_text(json.dumps(seeds,indent=2)+'\n');print('PASS36 unique code, whole bool1/BOOL4 stores, actual original registration identities')
