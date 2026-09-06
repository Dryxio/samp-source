from pathlib import Path
import sys,json,struct,hashlib
sys.path.insert(0,'tools')
from binary import PE,COFF
from rank_uncovered_candidates import accepted_code
D=Path('build/cp32-policy-ui-ob1');OUT=Path('build/agent-textdraw');p=PE(Path('private/samp.dll'));c=json.loads(Path('config/checkpoint32/rsa-commands-contract.json').read_text())
providers={r['anchor']:p.base+r['rva']+r.get('anchor_offset',0) for r in c['regions']};providers.update({n:v['reference_va'] for n,v in c['externals'].items()});providers['__except_list']=0
providers.update({'?NewVehicle@R5GameVehicleCreationView@@QAEPAVCVehicle@@HMMMMH@Z':p.base+0xa0250,'?tSettings@@3U_GAME_SETTINGS@@A':p.base+0x26dfe8,'?bHudScaleFix@@3_NA':p.base+0x117470,'?r5ShowCameraTarget@@3_NA':p.base+0x1189a8})
plan=[('closure_cmd_ui_settings',n,r,z) for n,r,z in [('cmdCameraTargetDebug',0x685e0,16),('cmdPageSize',0x685f0,120),('cmdNameTagStatus',0x68720,16),('cmdTimestamp',0x68730,93),('cmdAudioMsg',0x68790,101),('cmdLogUrls',0x68800,101),('cmdHudScaleFix',0x68870,57),('cmdMem',0x688b0,27),('cmdSetFrameLimit',0x688d0,134),('cmdHeadMove',0x68960,119)]]
plan += [('closure_cmd_rcon','cmdRcon',0x69030,216),('closure_cmd_player_skin','cmdPlayerSkin',0x68d00,109),('closure_cmd_create_vehicle','cmdCreateVehicle',0x68d70,308)]
plan=[(u,'?'+n+'@@YAXPAD@Z',r,z) for u,n,r,z in plan]+[('closure_cmd_ui_settings','?TogglePlayerStatus@R5NetworkNameTagStatusView@@QAEXXZ',0x8e90,118)]
for u,n,r,z in plan:providers[n]=p.base+r
objects={u:COFF(D/(u+'.obj')) for u,n,r,z in plan};maps={u:{} for u in objects};rows={};seeds=[]
def add(u,si,r,anchor=None,kind=None,duplicate=False):
 o=objects[u];s=o.sections[si-1];assert si not in maps[u] or maps[u][si]==r;maps[u][si]=r
 if anchor is None:anchor=next(x['name'] for x in o.symbols.values() if x['section']==si and x['name']!=s['name'])
 a=next(x for x in o.names[anchor] if x['section']==si)
 rows[(u,si)]=dict(unit=u,anchor=anchor,anchor_offset=a['value'],rva=r,size=s['size'],section=si,kind=kind or ('code' if s['name'].startswith('.text') else 'data'),duplicate_previous=duplicate)
for u,n,r,z in plan:
 s=next(x for x in objects[u].names[n] if x['section']>0);assert objects[u].sections[s['section']-1]['size']==z;add(u,s['section'],r,n);seeds.append(dict(unit=u,symbol=n,rva=r,size=z))
add('closure_cmd_rcon',53,0xe38c0);add('closure_cmd_rcon',54,0xfb770)
for u in ['closure_cmd_player_skin','closure_cmd_create_vehicle']:
 add(u,46,0x1010,duplicate=True);add(u,47,0xdfe80,duplicate=True);add(u,48,0xf7610,duplicate=True)
for u,n,r,z in [('closure_store_game_settings','?tSettings@@3U_GAME_SETTINGS@@A',0x26dfe8,1297),('closure_store_hud_scale_fix','?bHudScaleFix@@3_NA',0x117470,1),('closure_store_camera_target_debug','?r5ShowCameraTarget@@3_NA',0x1189a8,1)]:
 objects[u]=COFF(D/(u+'.obj'));maps[u]={};s=next(x for x in objects[u].names[n] if x['section']>0);assert objects[u].sections[s['section']-1]['size']==z;add(u,s['section'],r,n)
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
prior=accepted_code('evidence/checkpoint32/rsa-commands-acceptance.json');union=set()
for r in rows.values():
 if r['kind']=='code':union.update(range(r['rva'],r['rva']+r['size']))
assert len(prior)==377473;assert len(union-prior)==1556,len(union-prior)
out=dict(status='PASS_FULL_CODE_DATA_RELOCATIONS',run=D.name,baseline=len(prior),new_unique_code=1556,principal_code=1535,new_EH=21,source025_direct=0,duplicate_code_per_unit=115,duplicate_data_per_unit=36,regions=list(rows.values()),source_files={str(Path('client/saco')/(u+'.cpp')):hashlib.sha256((D/'client/saco'/(u+'.cpp')).read_bytes()).hexdigest() for u in objects},excluded=['Unused template/getter bodies and RakNet sentinel16 unreferenced; not seeded.','AddEntry538/PushBack24 are implemented unmatched downstream chat bodies, zero credit.','NewVehicle142 plus EH21 covered by separate root preflight, not included1556.'],provenance025='HudScale/Mem already in base; Rcon and Skin adapt R5 semantics; CreateVehicle changes final factory argument from source025 string to R5 flag0. All hybrid/base bodies receive0 direct025.')
(OUT/'cmd-ui-complete-reviewed-manifest.json').write_text(json.dumps(out,indent=2)+'\n');(OUT/'cmd-ui-complete-seeds.json').write_text(json.dumps(seeds,indent=2)+'\n');print(out['status'],len(rows),'sections',sum(len(r['bindings']) for r in rows.values()),'bindings',out['new_unique_code'],'unique')
