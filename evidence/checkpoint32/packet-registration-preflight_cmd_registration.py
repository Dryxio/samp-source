from pathlib import Path
import sys,json,struct,hashlib
sys.path.insert(0,'tools')
from binary import PE,COFF
from rank_uncovered_candidates import accepted_code
D=Path('build/cp32-cmd-registration-leaves-trial1');OUT=Path('build/agent-textdraw');p=PE(Path('private/samp.dll'));c=json.loads(Path('config/checkpoint32/remote-state-contract.json').read_text())
providers={r['anchor']:p.base+r['rva']+r.get('anchor_offset',0) for r in c['regions']};providers.update({n:v['reference_va'] for n,v in c['externals'].items()});providers['__except_list']=0
providers.update({'?dwStartQuitTick@@3KA':p.base+0x26eb34,'?iGameDebugType@@3HA':p.base+0x14ff50,'?dwDebugEntity1@@3KA':p.base+0x14ff54,'?dwDebugEntity2@@3KA':p.base+0x14ff58,'?r5RpcChat@@3HB':p.base+0xe58ec,'?pDeathWindow@@3PAVCDeathWindow@@A':p.base+0x26eb88})
plan=[('closure_cmd_registration_leaves','?'+n+'@@YAXPAD@Z',r,z) for n,r,z in [('cmdDefaultCmdProc',0x684e0,32),('cmdTestDeathWindow',0x68500,222),('cmdQuit',0x689e0,5),('cmdSelectVehicle',0x68eb0,24)]]
plan += [('closure_game_quit','?QuitGame@@YAXXZ',0xc3e80,54),('closure_local_player_say','?Say@CLocalPlayer@@QAEXPAD@Z',0x5a10,191),('closure_game_debug_entity','?GameDebugEntity@@YAXKKH@Z',0x9e0a0,30)]
for u,n,r,z in plan:providers[n]=p.base+r
objects={u:COFF(D/(u+'.obj')) for u,n,r,z in plan};maps={u:{} for u in objects};rows={};seeds=[]
def add(u,si,r,anchor=None,kind=None,duplicate=False):
 o=objects[u];s=o.sections[si-1];assert si not in maps[u] or maps[u][si]==r;maps[u][si]=r
 if anchor is None:anchor=next(x['name'] for x in o.symbols.values() if x['section']==si and x['name']!=s['name'])
 a=next(x for x in o.names[anchor] if x['section']==si)
 rows[(u,si)]=dict(unit=u,anchor=anchor,anchor_offset=a['value'],rva=r,size=s['size'],section=si,kind=kind or ('code' if s['name'].startswith('.text') else 'data'),duplicate_previous=duplicate)
for u,n,r,z in plan:
 s=next(x for x in objects[u].names[n] if x['section']>0);assert objects[u].sections[s['section']-1]['size']==z;add(u,s['section'],r,n);seeds.append(dict(unit=u,symbol=n,rva=r,size=z))
add('closure_local_player_say',51,0xe0040);add('closure_local_player_say',52,0xf7808)
for u,n,r,z in [('closure_store_quit_start_tick','?dwStartQuitTick@@3KA',0x26eb34,4),('closure_store_game_debug_type','?iGameDebugType@@3HA',0x14ff50,4),('closure_store_game_debug_entity1','?dwDebugEntity1@@3KA',0x14ff54,4),('closure_store_game_debug_entity2','?dwDebugEntity2@@3KA',0x14ff58,4),('closure_store_rpc_chat','?r5RpcChat@@3HB',0xe58ec,4)]:
 objects[u]=COFF(D/(u+'.obj'));maps[u]={};a=next(x for x in objects[u].names[n] if x['section']>0);assert objects[u].sections[a['section']-1]['size']==z;add(u,a['section'],r,n)
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
prior=accepted_code('evidence/checkpoint32/remote-state-acceptance.json');union=set()
for r in rows.values():
 if r['kind']=='code':union.update(range(r['rva'],r['rva']+r['size']))
assert len(prior)==384120;assert len(union-prior)==579,len(union-prior)
out=dict(status='CODE_EH_DATA_PASS_REQUIRED_DEATH_STORE_PENDING',run=D.name,baseline=len(prior),new_unique_code=579,principal_code=558,new_EH=21,source025_direct=54,regions=list(rows.values()),required_store=dict(unit='closure_store_death_window',anchor='?pDeathWindow@@3PAVCDeathWindow@@A',rva=0x26eb88,size=4,initial=0,accepted=False),excluded=['Unused inline helpers not credited.','SetupCommands593 and remaining font dependency not included.'],source_files={str(Path('client/saco')/(u+'.cpp')):hashlib.sha256((D/'client/saco'/(u+'.cpp')).read_bytes()).hexdigest() for u in objects})
from prepare_checkpoint32 import definition
ref=Path('/Users/salimtrouve/Documents/GitHub/samp-r5-preservation/references/michael-fa-samp/client')
for u,n,f in [('closure_cmd_registration_leaves','cmdSelectVehicle','cmdprocs.cpp'),('closure_game_debug_entity','GameDebugEntity','game/debug.cpp')]:
 actual=definition((D/'client/saco'/(u+'.cpp')).read_text(),n);reference=definition((ref/f).read_text(),n);assert actual==reference
sd=Path('build/cp32-death-window-store-trial1');so=COFF(sd/'closure_store_death_window.obj');sn='?pDeathWindow@@3PAVCDeathWindow@@A';ss=next(x for x in so.names[sn] if x['section']>0);sec=so.sections[ss['section']-1];assert ss['value']==0 and sec['size']==4 and sec['uninitialized'] and sec['reloc_count']==0
r=0x26eb88;containing=next(x for x in p.sections if x['rva']<=r and r+4<=x['rva']+x['virtual_size']);assert r>=containing['rva']+containing['size'];assert not any(r<=x<r+4 for x in p.relocations)
out['regions'].append(dict(unit='closure_store_death_window',anchor=sn,anchor_offset=0,rva=r,size=4,section=ss['section'],kind='data',whole_section=True,initial_bytes='00000000',bindings=[],sha256=hashlib.sha256(bytes(4)).hexdigest(),object_sha256=hashlib.sha256((sd/'closure_store_death_window.obj').read_bytes()).hexdigest(),object_run=sd.name))
out['required_store']['compiled_and_verified']=True;out['status']='PASS_FULL_CODE_EH_STORES_AND_RELOCATIONS';out['source_files']['client/saco/closure_store_death_window.cpp']=hashlib.sha256((sd/'client/saco/closure_store_death_window.cpp').read_bytes()).hexdigest()
(OUT/'cmd-registration-reviewed-manifest.json').write_text(json.dumps(out,indent=2)+'\n');(OUT/'cmd-registration-seeds.json').write_text(json.dumps(seeds,indent=2)+'\n');print(out['status'],len(rows),'sections',sum(len(r['bindings']) for r in rows.values()),'bindings',out['new_unique_code'],'unique')
