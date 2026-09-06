from pathlib import Path
import sys,json,struct,hashlib
sys.path.insert(0,'tools')
from binary import PE,COFF
from rank_uncovered_candidates import accepted_code
D=Path('build/cp32-vehicle-selection-trial1');OUT=Path('build/agent-textdraw');p=PE(Path('private/samp.dll'));c=json.loads(Path('config/checkpoint32/send-font-contract.json').read_text())
providers={r['anchor']:p.base+r['rva']+r.get('anchor_offset',0) for r in c['regions']};providers.update({n:v['reference_va'] for n,v in c['externals'].items()});providers['__except_list']=0
for row in json.loads(Path('build/agent-textdraw/selection-ped-reviewed-manifest.json').read_text())['regions']:providers[row['anchor']]=p.base+row['rva']+row.get('anchor_offset',0)
stores=[('initialized','?r5SelectionInitialized@@3HA',0x14ff98),('vehicle','?r5SelectionVehicle@@3PAVCVehicle@@A',0x14ff9c),('model','?r5SelectionModel@@3HA',0x1138ec),('controls','?r5SelectionControls@@3PAU_GTA_CONTROLSET@@A',0x14feb0),('camera','?r5SelectionCamera@@3PAVCCamera@@A',0x14feb4)]
for suffix,n,r in stores:providers[n]=p.base+r
plan=[('closure_vehicle_selection','?'+n+'@@YAXXZ',r,z) for n,r,z in [('GameBuildRecreateVehicle',0x9e0d0,59),('GameBuildSelectVehicle',0x9e110,886),('GameDebugDrawDebugScreens',0x9e490,20)]]
for u,n,r,z in plan:providers[n]=p.base+r
objects={u:COFF(D/(u+'.obj')) for u,n,r,z in plan};maps={u:{} for u in objects};rows={};seeds=[]
def add(u,si,r,anchor=None,kind=None,duplicate=False):
 o=objects[u];s=o.sections[si-1];assert si not in maps[u] or maps[u][si]==r;maps[u][si]=r
 if anchor is None:anchor=next(x['name'] for x in o.symbols.values() if x['section']==si and x['name']!=s['name'])
 a=next(x for x in o.names[anchor] if x['section']==si)
 rows[(u,si)]=dict(unit=u,anchor=anchor,anchor_offset=a['value'],rva=r,size=s['size'],section=si,kind=kind or ('code' if s['name'].startswith('.text') else 'data'),duplicate_previous=duplicate)
for u,n,r,z in plan:
 s=next(x for x in objects[u].names[n] if x['section']>0);assert objects[u].sections[s['section']-1]['size']==z;add(u,s['section'],r,n);seeds.append(dict(unit=u,symbol=n,rva=r,size=z))
add('closure_vehicle_selection',48,0x1010,duplicate=True);add('closure_vehicle_selection',49,0xdfe80,duplicate=True);add('closure_vehicle_selection',50,0xf7610,duplicate=True)
for suffix,n,r in stores:
 u='closure_store_vehicle_selection_'+suffix;objects[u]=COFF(D/(u+'.obj'));maps[u]={};a=next(x for x in objects[u].names[n] if x['section']>0);assert objects[u].sections[a['section']-1]['size']==4;add(u,a['section'],r,n)
# Discover only actual referenced, complete nonrelocating string/float sections. Providers are independently mapped above/baseline.
for (u,si),row in list(rows.items()):
 o=objects[u];sec=o.sections[si-1];r=row['rva']
 for i in range(sec['reloc_count']):
  off,idx,k=struct.unpack_from('<IIH',o.data,sec['reloc_ptr']+i*10);sym=o.symbols[idx];a=struct.unpack_from('<I',sec['bytes'],off)[0]
  if sym['section']>0 and sym['section'] not in maps[u] and (sym['name'].startswith('??_C@') or sym['name'].startswith('__real@')):
   v=struct.unpack('<I',p.read(r+off,4))[0];target=v if k==6 else (v+p.base+r+off+4)&0xffffffff;dr=target-p.base-a-sym['value'];ds=o.sections[sym['section']-1];assert ds['reloc_count']==0;assert ds['bytes']==p.read(dr,ds['size']);add(u,sym['section'],dr,sym['name'])
for (u,si),row in rows.items():
 o=objects[u];sec=o.sections[si-1];r=row['rva'];raw=bytearray(sec['bytes'] if not sec['uninitialized'] else bytes(sec['size']));bs=[]
 for i in range(sec['reloc_count']):
  off,idx,k=struct.unpack_from('<IIH',o.data,sec['reloc_ptr']+i*10);sym=o.symbols[idx];n=sym['name'];a=struct.unpack_from('<I',raw,off)[0]
  sv=p.base+maps[u][sym['section']]+sym['value'] if sym['section'] in maps[u] else providers[n]
  target=(sv+a)&0xffffffff;v=struct.unpack('<I',p.read(r+off,4))[0];actual=v if k==6 else (v+p.base+r+off+4)&0xffffffff;assert target==actual,(u,n,hex(r+off),hex(target),hex(actual));struct.pack_into('<I',raw,off,(target-(p.base+r+off+4 if k==20 else 0))&0xffffffff);bs.append(dict(symbol=n,site_rva=r+off,symbol_va=sv,addend=a,target_va=target,kind=k))
 
 try: expected=p.read(r,len(raw))
 except ValueError:
  containing=next(s for s in p.sections if s['rva']<=r and r+len(raw)<=s['rva']+s['virtual_size'])
  assert r>=containing['rva']+containing['size'];expected=bytes(len(raw))
 assert raw==expected,(u,hex(r),'bytes')
 assert {b['site_rva'] for b in bs if b['kind']==6 and b['symbol']!='__except_list'}=={x for x in p.relocations if r<=x<r+len(raw)},(u,hex(r),'PEreloc')
 row.update(bindings=bs,sha256=hashlib.sha256(raw).hexdigest(),object_sha256=hashlib.sha256((D/(u+'.obj')).read_bytes()).hexdigest(),whole_section=True)
for row in rows.values():
 if row['duplicate_previous']:
  old=[r for r in c['regions'] if r['rva']==row['rva'] and r['size']==row['size']]
  assert old and all(r['sha256']==row['sha256'] for r in old)
  row['previous_identical_owners']=[r['unit'] for r in old]
prior=accepted_code('evidence/checkpoint32/send-font-acceptance.json');union=set()
for r in rows.values():
 if r['kind']=='code':union.update(range(r['rva'],r['rva']+r['size']))
assert len(prior)==393482;assert len(union-prior)==965,len(union-prior)
from prepare_checkpoint32 import definition
ref=Path('/Users/salimtrouve/Documents/GitHub/samp-r5-preservation/references/michael-fa-samp/client/game/debug.cpp');actual=definition((D/'client/saco/closure_vehicle_selection.cpp').read_text(),'GameDebugDrawDebugScreens');reference=definition(ref.read_text(),'GameDebugDrawDebugScreens');assert actual==reference
out=dict(status='PASS_FULL_CODE_EH_DATA_RELOCATIONS',run=D.name,baseline=len(prior),new_unique_code=965,source025_direct=20,direct025_definition_sha256=hashlib.sha256(actual.encode()).hexdigest(),source025_reference_sha256=hashlib.sha256(ref.read_bytes()).hexdigest(),regions=list(rows.values()),source_files={str(Path('client/saco')/(u+'.cpp')):hashlib.sha256((D/'client/saco'/(u+'.cpp')).read_bytes()).hexdigest() for u in objects},profile='Ob2 originalfirstRecreateinline and fourothercalls preserved, entire886 verified.',real_types='OriginalGTA_CONTROLSET previous/current offsets1C/4C,20/50,1E/4E; actualfullCVehicle nativeentity40 andGTAid44; destructorvirtualslot0 uses completeacceptedVFT. Factory view identical to acceptedowner, neverallocated.',new_ped_dependency='ToggleControllable178+PutDirectly381 exact reviewed separately, not included965.',duplicates='FindPlayer94/EH21/FuncInfo36 wholephysicalcopies reviewed againstpreviousacceptedowners, zero newcredit. UnusedGetCamera4 excluded.',floating='Fulloriginal code includes inline5.0,500.0,502.0,-4.0,0.03F; allstaticliteral5.0/1.0 full4-byte objects verified, no reinterpret-double error.',native_behavior='Control and camera pointers captured from actualproviders; no runtimegameoperationexecuted.')
(OUT/'vehicle-selection-reviewed-manifest.json').write_text(json.dumps(out,indent=2)+'\n');(OUT/'vehicle-selection-seeds.json').write_text(json.dumps(seeds,indent=2)+'\n');print(out['status'],len(rows),'sections',sum(len(r['bindings']) for r in rows.values()),'bindings',out['new_unique_code'],'unique')
