import json,sys,struct
from pathlib import Path
sys.path.insert(0,'tools')
from binary import COFF,PE
from verify_checkpoint2 import import_slots
old=json.load(open('config/checkpoint32/map-trailer-contract.json'));new=json.load(open('build/actor-contract-draft.json'));prior={(r['rva'],r['size'],r['kind']):r for r in old['regions']}
pe=PE('private/samp.dll');run=Path('build/cp32-death-vehicle-linked2');objs={p.stem:COFF(p) for p in run.glob('*.obj')}
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
seeds=json.load(open('config/checkpoint32/death-vehicle-seeds.json'))
spec={'closure_preview_entity_render':[(0x9fb20,138)],'closure_preview_quaternion_utilities':[(0xb57e0,68),(0xb58a0,46),(0xb58d0,57)],'closure_death_message_providers':[(0x3f20,33),(0x16180,22),(0x175c0,72)],'closure_rpc_vehicle_params_components':[(0x19e60,204),(0x1c5c0,213)],'closure_vehicle_params_components_providers':[(0xb7840,48),(0x1eb30,84),(0xb80b0,61)],'deathwindow':[(0x6a6b0,5)],'closure_world':[(0x9ec80,82),(0x9fd00,91)]}
codes={}
for u,entries in spec.items():
 for rva,size in entries:
  name=next(n for n,v in seeds[u].items() if v==rva);codes[rva]=(size,name);ids[name]=pe.base+rva
codes[0xe0ff0]=(21,'__ehhandler$'+codes[0x19e60][1]);codes[0xe1430]=(21,'__ehhandler$'+codes[0x1c5c0][1])
handlers={0xf8a14:0xe0ff0,0xf8edc:0xe1430};strings={};literals={}
from binary import sha
local={};seen=set();count=0
for r in new['regions']:
 if (r['rva'],r['size'],r['kind']) in prior:continue
 count+=1;assert r['unit'] in spec or r['unit']=='closure_store_remove_vehicle_component'
 o=objs[r['unit']];sec=o.sections[r['section']-1];assert sec['size']==r['size'] or (r['kind']=='script-command' and r['size']==18 and not sec['reloc_count'])
 if r['kind']=='code':
  size,name=codes[r['rva']];assert r['size']==size and r['anchor'].startswith(name);seen.add(r['rva'])
 elif r['rva'] in strings:assert (r['size'],sha(pe.read(r['rva'],r['size'])))==strings[r['rva']]
 elif r['rva'] in handlers:
  assert r['size']==36 and r['anchor_offset']==8
  assert pe.read(r['rva'],36)==struct.pack('<IIIIIIIII',0xffffffff,pe.base+handlers[r['rva']],0x19930520,1,pe.base+r['rva'],0,0,0,0)
 elif r['rva'] in literals:
  size,expected=literals[r['rva']];assert r['size']==size and pe.read(r['rva'],size)==expected.ljust(size,b'\0')
 elif r['rva']==0xed678:
  assert r['size']==18 and r['anchor']=='?r5RemoveVehicleComponent@@3USCRIPT_COMMAND@@B'
  assert pe.read(r['rva'],18)==struct.pack('<H',0x06e8)+b'ii'+bytes(14)
 else:raise AssertionError(('Unreviewed new data',r))
 for ss in o.names.values():
  for sy in ss:
   if sy['section']==r['section'] and r['offset']<=sy['value']<r['offset']+r['size']:
    address=pe.base+r['rva']+sy['value']-r['offset'];local[(r['unit'],sy['name'])]=address
    if sy['storage']==2:ids[sy['name']]=address
 ids[r['anchor']]=pe.base+r['rva']+r['anchor_offset']
assert seen==set(codes) and count==20,(count,seen)
for n,e in new['externals'].items():
 if n in old['externals']:
  p=old['externals'][n];assert {k:v for k,v in p.items() if k not in ('import','library','chunks')}=={k:v for k,v in e.items() if k not in ('import','library','chunks')}
  if 'chunks' in e:assert e['chunks']==p['chunks']
  for k in ('import','library','chunks'):
   if k in p:e[k]=p[k]
 else:raise AssertionError(('Unexpected external',n,e))
 ids[n]=e['reference_va']
for u,aliases in old['weak_aliases'].items():
 expected=dict(aliases)
 if u=='closure_world':expected.update({'_r5_EntitySetMatrix_thiscall_assembly_only':'?SetMatrix@CEntity@@QAEXU_MATRIX4X4@@@Z','_r5_EntityUpdateRw_thiscall_assembly_only':'?FUNC_1009EC80@CEntity@@QAEXXZ'})
 assert new['weak_aliases'][u]==expected,(u,new['weak_aliases'][u],expected)
for aliases in new['weak_aliases'].values():
 for alias,canonical in aliases.items():ids[alias]=ids[canonical]
for r in new['regions']:
 p=prior.get((r['rva'],r['size'],r['kind']))
 if p:
  assert r['sha256']==p['sha256'];assert [(f['offset'],f['kind'],f['reference_va']) for f in r['fixups']]==[(f['offset'],f['kind'],f['reference_va']) for f in p['fixups']];r['implementation']=p['implementation'];continue
 for f in r['fixups']:assert local.get((r['unit'],f['symbol']),ids.get(f['symbol']))==f['reference_va'],(r['anchor'],f)
 r['implementation']='Complete C++ source function and compiler EH; DisplayGameText retains existing symbolic native call' if r['kind']=='code' else 'Complete source string, actual global pointer/integer storage, SCRIPT_COMMAND or unwind metadata'
assert len(new['pending'])==5
for r in new['pending']:
 p=next(p for p in old['pending'] if p['anchor']==r['anchor']);r['reference_function_code_size']=p['reference_function_code_size']
 if 'limitation' in p:r['limitation']=p['limitation']
new['status']='REVIEWED';new['sdk']=json.load(open(run/'link.json',encoding='utf-8-sig'))['sdk'];assert all(new['sdk'][n]==h for n,h in old['sdk'].items())
new['review']=dict(old['review']);new['review'].update(scope='Vehicle params/components652, death name/color providers132, entity render138/quaternion171, complete matrix update173.',adaptations='Normal C++ provider/RPC source, corrected actual GetAt access; RemoveComponent61 uses full symbolic branch-order exception after three C++ layouts. EntityRender138 exception after exact-size C++ register mismatch. World Update82/SetMatrixAndUpdate91 are symbolic after old C++80/89, unique original owners retained. True thiscall aliases only, exact member providers with no wrapper bytes. No new headers or storage except full component descriptor18.',verification='Whole functions/data, two EH21/map36 pairs, all actual providers/native ABI and relocations. New matrix aliases resolved to actual closure_world SetMatrix177/Update82 symbols and callsites. All prior regions preserved, five pending remain. Ten controls and actor ABI required.',reference_credit='025 VehicleParams204+Assign84+Door48+RemoveComponentRPC213 =549, adapted R5 fields/WORD argument. DeathRPC493 failed490 and is excluded; name/color providers132 were base/reconstruction. Other new functions R5/native reconstruction, no direct025 credit.',initialization_limitation='Views only access existing pool/player/vehicle objects, identical definitions embedded in twoCPPs to avoid changing shared headers; no allocation/ownership claims. No complete DeathRPC or ped/vehicle preview constructor claim.',matrix_update='Whole Update82 calls actual CMatrix::UpdateRW59AD70 and nativeEntity::UpdateRwFrame532B00; SetMatrixAndUpdate91 uses native Remove slot3 and Add()slot2, full64-byte argument forwarding to actual SetMatrix9EBC0/177 then real Update82. Five remaining pending exclude unrelated Euler/GUI/font functions.')
for r in new['regions']:
 if (r['rva'],r['size'],r['kind']) not in prior:
  r['implementation']='Complete normal C++ with established native ABI bridge as applicable; compiler EH' if r['kind']=='code' else 'Complete actual script command or unwind metadata'
  if r['rva'] in (0x9fb20,0x9ec80,0x9fd00,0xb80b0):r['implementation']='Explicit complete symbolic exception after bounded C++ failures; named fields/local slots/real native calls, no raw bytes or padding'
new['review']['linker_alias_provider_evidence']=json.load(open('build/cp32-death-vehicle-linked2-aliases.json'))
Path('config/checkpoint32/death-vehicle-contract.json').write_text(json.dumps(new,indent=2)+'\n')
from verify_actor_closure import Gate
g=Gate('cp32-death-vehicle-linked2',contract_path='config/checkpoint32/death-vehicle-contract.json');g.bind()
new['review']['associative_comdat_bindings']={'run':'cp32-death-vehicle-linked2','bindings':g.associative_bindings,'negative_controls':json.load(open('build/associative-binding-controls.json')),'scope':'Previously tested strict associative-owner binding; all actual bytes and relocations still verified. Duplicate parent/EH gives no additional coverage.'}
Path('config/checkpoint32/death-vehicle-contract.json').write_text(json.dumps(new,indent=2)+'\n')
