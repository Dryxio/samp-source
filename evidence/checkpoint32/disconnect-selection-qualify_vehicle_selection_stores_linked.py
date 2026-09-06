from pathlib import Path
import sys,re,json,hashlib
sys.path.insert(0,'tools');from binary import COFF,PE
D=Path('build/cp32-disconnect-selection-linked1');p=PE(D/'closure.dll');maps={}
for l in (D/'closure.map').read_text(errors='replace').splitlines():
 m=re.match(r'\s*[0-9a-fA-F]{4}:[0-9a-fA-F]{8}\s+(\S+)\s+([0-9a-fA-F]{8})\s+(?:f\s+)?(?:i\s+)?(\S+)\s*$',l)
 if m:maps.setdefault(m[1],[]).append((int(m[2],16),m[3]))
plan=[('?r5SelectionInitialized@@3HA', 'closure_store_vehicle_selection_initialized', 4, 0), ('?r5SelectionVehicle@@3PAVCVehicle@@A', 'closure_store_vehicle_selection_vehicle', 4, 0), ('?r5SelectionModel@@3HA', 'closure_store_vehicle_selection_model', 4, 400), ('?r5SelectionControls@@3PAU_GTA_CONTROLSET@@A', 'closure_store_vehicle_selection_controls', 4, 0), ('?r5SelectionCamera@@3PAVCCamera@@A', 'closure_store_vehicle_selection_camera', 4, 0)];owners={n:[] for n,u,z,b in plan}
for f in D.glob('*.obj'):
 o=COFF(f)
 for n in owners:
  for s in o.names.get(n,[]):
   if s['section']>0 and s['storage']==2:owners[n].append((f.name,s,o.sections[s['section']-1]))
rows=[]
for n,u,z,b in plan:
 hits=owners[n];assert len(hits)==1 and hits[0][0]==u+'.obj',(n,[(h[0]) for h in hits]);_,s,sec=hits[0];assert s['value']==0 and sec['size']==z and sec['reloc_count']==0;assert maps[n]==[(maps[n][0][0],u+'.obj')];va=maps[n][0][0];r=va-p.base
 try:data=p.read(r,z)
 except ValueError:
  containing=next(x for x in p.sections if x['rva']<=r and r+z<=x['rva']+x['virtual_size']);assert r>=containing['rva']+containing['size'];data=bytes(z)
 assert data==b.to_bytes(z,'little');assert not any(r<=a<r+z for a in p.relocations)
 rows.append(dict(symbol=n,sole_owner=u,linked_va=va,whole_size=z,initial_byte=b,object_sha256=hashlib.sha256((D/(u+'.obj')).read_bytes()).hexdigest()))
result=dict(status='PASS_SOLE_REAL_STORES_AND_LINKED_INITIALIZATION',run=D.name,owners=rows);Path('build/agent-textdraw/vehicle-selection-stores-linked-review.json').write_text(json.dumps(result,indent=2)+'\n');print(result['status'])
