import sys,json,struct
from pathlib import Path
import hashlib
sys.path.insert(0,'tools');from binary import PE,COFF,sha,u32
from verify_checkpoint2 import relocs
from verify_actor_closure import initial_bytes
from rank_uncovered_candidates import accepted_code
p=PE('private/samp.dll');c=json.load(open('config/checkpoint32/trial30e-one-contract.json'));run=Path('build/cp32-trial30e-queries1');objs={'closure_vehicle_damage_queries':COFF(run/'closure_vehicle_damage_queries.obj'),'closure_vehicle_drive_queries':COFF('build/cp32-trial30e-drive1/closure_vehicle_drive_queries.obj')}
providers={r['anchor']:p.base+r['rva']+r.get('anchor_offset',0) for r in c['regions']};providers.update({n:e['reference_va'] for n,e in c['externals'].items()});providers['__except_list']=0
plan=[('closure_vehicle_damage_queries', '?GetPanelDamageStatus@R5VehicleDamageQueriesView@@QAEKXZ', 752512, 39), ('closure_vehicle_damage_queries', '?GetDoorDamageStatus@R5VehicleDamageQueriesView@@QAEKXZ', 752560, 39), ('closure_vehicle_damage_queries', '?GetLightDamageStatus@R5VehicleDamageQueriesView@@QAEEXZ', 752608, 39), ('closure_vehicle_drive_queries', '?IsLocalPlayerVehicle@R5VehicleDriveQueriesView@@QAEHXZ', 750624, 52), ('closure_vehicle_drive_queries', '?UpdateLastDrivenTime@R5VehicleDriveQueriesView@@QAEHXZ', 751728, 47), ('closure_vehicle_drive_queries', '?GetTireStatus@R5VehicleDriveQueriesView@@QAEEXZ', 752176, 141)]
todo=[];seeds=[];rows=[];seen={}
for u,n,r,z in plan:
 providers[n]=p.base+r;s=next(x for x in objs[u].names[n] if x['section']>0);assert objs[u].sections[s['section']-1]['size']==z,(n,objs[u].sections[s['section']-1]['size'],z);todo.append((u,s['section'],r));seeds.append(dict(unit=u,symbol=n,rva=r,size=z))
while todo:
 u,si,r=todo.pop();k=(u,si)
 if k in seen:assert seen[k]==r;continue
 seen[k]=r;o=objs[u];sec=o.sections[si-1];raw=bytearray(sec['bytes'] if sec['bytes'] else bytes(sec['size']));ref=initial_bytes(p,r,sec['size']);bindings=[]
 for f in relocs(o,si):
  off=f['offset'];s=f['symbol'];a=u32(raw,off);target=u32(ref,off)
  if f['kind']==20:target=(target+p.base+r+off+4)&0xffffffff
  va=(target-a)&0xffffffff
  if s['section']>0:todo.append((u,s['section'],va-p.base-s['value']))
  else:assert providers[s['name']]==va,(u,s['name'],hex(va),hex(providers.get(s['name'],0)))
  value=(target-(p.base+r+off+4 if f['kind']==20 else 0))&0xffffffff;struct.pack_into('<I',raw,off,value);bindings.append(dict(symbol=s['name'],site_rva=r+off,kind=f['kind'],target_va=target))
 assert bytes(raw)==ref,(u,hex(r))
 assert {x for x in p.relocations if r<=x<r+sec['size']}=={b['site_rva'] for b in bindings if b['kind']==6 and b['symbol']!='__except_list'}
 anchor=next(x for x in o.symbols.values() if x['section']==si and x['name']!=sec['name']);rows.append(dict(unit=u,section=si,anchor=anchor['name'],anchor_offset=anchor['value'],rva=r,size=sec['size'],kind='code' if sec['flags']&0x20 else 'data',sha256=sha(ref),bindings=bindings))
covered=accepted_code('evidence/checkpoint32/trial30e-one-acceptance.json');new={a for r in rows if r['kind']=='code' for a in range(r['rva'],r['rva']+r['size'])}-covered;assert len(new)==357
out=dict(status='PASS_FULL_SOURCE_REGIONS_ALL_COFF_PE_TARGETS',run=run.name,baseline=len(covered),new_unique_code=len(new),regions=rows,source025_direct=0,object_sha256={u:sha(((run if u=='closure_vehicle_damage_queries' else Path('build/cp32-trial30e-drive1'))/(u+'.obj')).read_bytes()) for u in objs},source_sha256={u:sha(((run if u=='closure_vehicle_damage_queries' else Path('build/cp32-trial30e-drive1'))/('client/saco/'+u+'.cpp')).read_bytes()) for u in objs},typed_layout_note='Three025 damagequeries117 and025 last-driven47 plusR5 localvehicle52/tiremask141. Pointer4c, timestamp69/flag6d, actualexisting engine-pool/subtype/import targets. No newstorageCRT.')
Path('build/trial30e/queries-reviewed-manifest.json').write_text(json.dumps(out,indent=2)+'\n');Path('build/trial30e/queries-seeds.json').write_text(json.dumps(seeds,indent=2)+'\n');print('PASS',len(rows),'whole regions',len(new),'unique',sum(len(r['bindings']) for r in rows),'fixups')
