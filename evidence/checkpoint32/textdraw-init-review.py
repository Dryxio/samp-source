import json,sys,struct
from pathlib import Path
sys.path.insert(0,'tools')
from binary import COFF,PE
from verify_checkpoint2 import import_slots
old=json.load(open('config/checkpoint32/selection-model-motion-contract.json'));new=json.load(open('build/actor-contract-draft.json'));prior={(r['rva'],r['size'],r['kind']):r for r in old['regions']}
pe=PE('private/samp.dll');run=Path('build/cp32-textdraw-init-linked1');objs={p.stem:COFF(p) for p in run.glob('*.obj')}
ids={f['symbol']:f['reference_va'] for r in old['regions'] for f in r['fixups']};ids.update({r['anchor']:pe.base+r['rva']+r['anchor_offset'] for r in old['regions']});ids.update({n:e['reference_va'] for n,e in old['externals'].items()})
# Recover member identities only from previously accepted complete source objects.
for r in old['regions']:
 o=objs.get('closure_gui_widget_lifetime' if r['unit']=='closure_gui_control_lifetime' else 'closure_dxut_state' if r['unit'] in ('closure_dxut_settings','closure_dxut_cursor','closure_dxut_multimon','closure_dxut_device') else r['unit'])
 if not o or r['kind']=='code':continue
 syms=o.names.get(r['anchor'],[]);sy=next((s for s in syms if s['section']>0),None)
 if not sy:continue
 for ss in o.names.values():
  for s in ss:
   if s['section']==sy['section'] and s['storage']==2 and not s['name'].startswith('??_C@'):
    delta=s['value']-sy['value']+r['anchor_offset']
    if 0<=delta<r['size']:ids[s['name']]=pe.base+r['rva']+delta
seeds=json.load(open('config/checkpoint32/textdraw-init-seeds.json'))
spec={'closure_textdraw_texture_loader':[(0xb2cd0,630)],'closure_textdraw_set_text':[(0xb2f60,298)],'closure_textdraw_constructor':[(0xb36e0,402)],'closure_textdraw_pool_new':[(0x1e910,155)],'closure_rpc_vehicle_motion':[(0x1eb90,42),(0x18950,367)],'closure_rpc_game_text':[(0xa0ce0,120),(0x198f0,260)],'closure_pregame_patches':[(0xaa470,76),(0xaa710,673)],'closure_game_init_buffer':[(0xa0890,72)]}
codes={}
for u,entries in spec.items():
 for rva,size in entries:
  name=next(n for n,v in seeds[u].items() if v==rva);codes[rva]=(size,name);ids[name]=pe.base+rva
handlers={0xf923c:0xe1730,0xf86fc:0xe0d30,0xf893c:0xe0f30}
for rva,parent in [(0xe1730,0x1e910),(0xe0d30,0x18950),(0xe0f30,0x198f0)]:codes[rva]=(21,'__ehhandler$'+codes[parent][1])
strings={int(r['rva'],16):(r['size'],r['sha256']) for r in json.load(open('build/agent-textdraw/textdraw-construction-next-manifest.json'))['strings']}
from binary import sha
from verify_checkpoint2 import map_symbols
maps=map_symbols(run/'closure.map')
assert pe.read(0xe5cc8,3)==b'%X\0'
ids['??_C@_02EMFKHFLK@?$CFX?$AA@']=pe.base+0xe5cc8
local={};seen=set();count=0
for r in new['regions']:
 if (r['rva'],r['size'],r['kind']) in prior:continue
 count+=1;assert r['unit'] in spec or r['unit'] in ('closure_store_game_text_message','closure_store_system_memory','closure_store_streaming_memory')
 o=objs[r['unit']];sec=o.sections[r['section']-1];assert sec['size']==r['size'] or (r['kind']=='script-command' and r['size']==18 and not sec['reloc_count'])
 if r['kind']=='code':
  size,name=codes[r['rva']];assert r['size']==size and r['anchor'].startswith(name);seen.add(r['rva'])
 elif r['rva'] in strings:assert (r['size'],sha(pe.read(r['rva'],r['size'])))==strings[r['rva']]
 elif r['rva'] in handlers:
  assert r['size']==36 and r['anchor_offset']==8
  assert pe.read(r['rva'],36)==struct.pack('<IIIIIIIII',0xffffffff,pe.base+handlers[r['rva']],0x19930520,1,pe.base+r['rva'],0,0,0,0)
 elif r['rva'] in (0x150334,0x26b3d8,0x26b3dc):assert r['kind']=='zero' and r['size']==4
 elif r['rva']==0xec724:assert r['size']==18 and pe.read(r['rva'],18)==struct.pack('<H',0xbe)+bytes(16)
 else:
  texts={0xecd8c:b'title\0',0xec7d4:b'Version Error\0',0xec788:b"I can't determine your GTA version.\r\nSA-MP only supports GTA:SA v1.0 USA/EU\0"}
  t=texts[r['rva']];assert pe.read(r['rva'],r['size'])==t.ljust(r['size'],b'\0') and r['size']-len(t)<4
 for ss in o.names.values():
  for sy in ss:
   if sy['section']==r['section'] and r['offset']<=sy['value']<r['offset']+r['size']:
    address=pe.base+r['rva']+sy['value']-r['offset'];local[(r['unit'],sy['name'])]=address
    if sy['storage']==2:ids[sy['name']]=address
 ids[r['anchor']]=pe.base+r['rva']+r['anchor_offset']
assert seen==set(codes) and count==31,(count,seen)
for n,e in new['externals'].items():
 if n in old['externals']:
  p=old['externals'][n];assert {k:v for k,v in p.items() if k not in ('import','library','chunks')}=={k:v for k,v in e.items() if k not in ('import','library','chunks')}
  if 'chunks' in e:assert e['chunks']==p['chunks']
  for k in ('import','library','chunks'):
   if k in p:e[k]=p[k]
 elif n=='_strncmp':
  assert e==dict(kind='crt',reference_va=0x100cb030,size=57,sha256='591725832adb8343cdb0b00cb336d17f54bf82c34875ede3bece382072dd0644')
  assert any(owner.upper()=='LIBCMT:STRNCMP.OBJ' for _,owner in maps[n])
 else:
  expected={'__imp__GlobalMemoryStatusEx@4':(0x100e5180,('kernel32.dll','GlobalMemoryStatusEx')),'__imp__ExitProcess@4':(0x100e5188,('kernel32.dll','ExitProcess'))}
  va,identity=expected[n];assert e==dict(kind='import',reference_va=va);assert tuple(import_slots(pe)[va])==identity;e['import']=list(identity)
 ids[n]=e['reference_va']
for u,aliases in old['weak_aliases'].items():assert new['weak_aliases'][u]==aliases
for aliases in new['weak_aliases'].values():
 for alias,canonical in aliases.items():ids[alias]=ids[canonical]
for r in new['regions']:
 p=prior.get((r['rva'],r['size'],r['kind']))
 if p:
  assert r['sha256']==p['sha256'];assert [(f['offset'],f['kind'],f['reference_va']) for f in r['fixups']]==[(f['offset'],f['kind'],f['reference_va']) for f in p['fixups']];r['implementation']=p['implementation'];continue
 for f in r['fixups']:assert local.get((r['unit'],f['symbol']),ids.get(f['symbol']))==f['reference_va'],(r['anchor'],f)
 r['implementation']='Complete C++ source function and compiler EH; DisplayGameText retains existing symbolic native call' if r['kind']=='code' else 'Complete source string, actual global pointer/integer storage, SCRIPT_COMMAND or unwind metadata'
assert len(new['pending'])==8
for r in new['pending']:
 p=next(p for p in old['pending'] if p['anchor']==r['anchor']);r['reference_function_code_size']=p['reference_function_code_size']
 if 'limitation' in p:r['limitation']=p['limitation']
new['status']='REVIEWED';new['sdk']=json.load(open(run/'link.json',encoding='utf-8-sig'))['sdk'];assert all(new['sdk'][n]==h for n,h in old['sdk'].items())
new['review']=dict(old['review']);new['review'].update(scope='Textdraw construction1506, Vehicle/GameText831 and actual pregame/init821.',adaptations='Textdraw SetText local declaration order; loader align8 arrays and explicit memset65. Full allocated construction size0x9D6 with R5 layout assertions. Original GameText four-byte memset preserved; actual InitGame calloc513 closes pointer initialization. Pregame patch data comes from existing source; never executed.',verification='Whole31 newly placed regions and all previous regions; actual provider identities independently audited, new strncmp pinned LIBCMT:strncmp.obj and actual Windows imports. Complete relocation comparisons, ten standard negative controls and actor ABI required.',reference_credit='824 direct adapted025 bytes: PoolNew155, VehicleLookup42, VehicleVelocity367, GameTextRPC260. Constructor402 hybrid qualitative contribution; other bodies already supplied by base. EH excluded from direct025 counter.')
new['review']['linker_alias_provider_evidence']=json.load(open('build/cp32-textdraw-init-linked1-aliases.json'))
Path('config/checkpoint32/textdraw-init-contract.json').write_text(json.dumps(new,indent=2)+'\n')
from verify_actor_closure import Gate
g=Gate('cp32-textdraw-init-linked1',contract_path='config/checkpoint32/textdraw-init-contract.json');g.bind()
new['review']['associative_comdat_bindings']={'run':'cp32-textdraw-init-linked1','bindings':g.associative_bindings,'negative_controls':json.load(open('build/associative-binding-controls.json')),'scope':'Previously tested strict associative-owner binding; all actual bytes and relocations still verified. Duplicate parent/EH gives no additional coverage.'}
Path('config/checkpoint32/textdraw-init-contract.json').write_text(json.dumps(new,indent=2)+'\n')
