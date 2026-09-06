import json,sys,struct
from pathlib import Path
sys.path.insert(0,'tools')
from binary import COFF,PE
from verify_checkpoint2 import import_slots
old=json.load(open('config/checkpoint32/huffman-ped-contract.json'));new=json.load(open('build/actor-contract-draft.json'));prior={(r['rva'],r['size'],r['kind']):r for r in old['regions']}
pe=PE('private/samp.dll');run=Path('build/cp32-vehicle-statistics-linked1');objs={p.stem:COFF(p) for p in run.glob('*.obj')}
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
seeds=json.load(open('config/checkpoint32/vehicle-statistics-seeds.json'))
sizes={0x6c3c0:686,0x36610:2338,0x1120:40,0x1150:29,0xb7c90:83,0xb8a10:40,0xb8a40:40,0xb7ee0:223,0xb42b0:65,0xb88f0:286,0xb8b50:30,0xb83d0:1308,0xb5830:110,0xb1410:32,0x9f960:101,0x9fa30:35}
codes={}
units=['closure_vendor_statistics','closure_vehicle_lifetime_helpers','closure_vehicle_trailer_pool','closure_vehicle_destructor','closure_vehicle_constructor','closure_entity_native_queries','closure_vehicle_preview_render']
for u in units:
 for n,rva in seeds[u].items():codes[rva]=(sizes[rva],n);ids[n]=pe.base+rva
codes[0xe3980]=(21,None);codes[0xe3fd0]=(18,None);codes[0xe3ff0]=(18,None)
handlers={0xfb848:0xe3980,0xfbf3c:0xe3fd0,0xfbf60:0xe3ff0}
from binary import sha
from verify_actor_closure import initial_bytes
stats=json.load(open('build/agent-independent/vendor-statistics-compiled-preflight.json'))
data={r['rva']:(r['size'],r['sha256']) for r in stats['data_regions']}
for rva,size,payload in [(0xec314,4,struct.pack('<f',0.25)),(0xed468,4,struct.pack('<f',180)),(0xe6118,4,struct.pack('<f',2)),(0xed6fc,42,b"Warning: couldn't create vehicle type: %u\0"),(0xed6dc,29,b'Warning: bad train carriages\0')]:
 assert len(payload)==size,(rva,size,len(payload));data[rva]=(size,sha(payload))
for r in json.load(open('build/agent-textdraw/vehicle-preview-manifest.json'))['literal_payloads']:data[int(r['rva'],16)]=(4,sha(bytes.fromhex(r['hex'])))
commands={0xed4e8:(0xa5,b'ifffv'),0xed4fc:(0xa6,b'i'),0xed5c4:(0x6d8,b'ifffiv'),0xed5d8:(0x7bd,b'i'),0xed5ec:(0x9c4,b'ii'),0xed614:(0x7ff,b'ii'),0xed68c:(0x53f,b'ii'),0xed538:(0x164,b'i'),0xed5b0:(0x362,b'ifff')}
for rva,(opcode,args) in commands.items():data[rva]=(18,sha(struct.pack('<H',opcode)+args+bytes(16-len(args))))
zeros={0x26ebe0:4,0x15180c:4,0x26dfcc:4,0x26dfd0:4,0x118be0:4,0x118be8:8,0x118bf0:8,0x26df9c:12,0x26df90:12,0x26cf7c:4}
local={};seen=set();count=0
for r in new['regions']:
 if (r['rva'],r['size'],r['kind']) in prior:continue
 count+=1;o=objs[r['unit']];sec=o.sections[r['section']-1]
 assert sec['size']==r['size'] or (r['kind']=='script-command' and r['size']==18 and not sec['reloc_count'])
 if r['kind']=='code':
  size,name=codes[r['rva']];assert r['size']==size and (name is None or r['anchor'].startswith(name));seen.add(r['rva'])
 elif r['rva'] in handlers:
  assert r['size']==36 and r['anchor_offset']==8
  assert pe.read(r['rva'],36)==struct.pack('<IIIIIIIII',0xffffffff,pe.base+handlers[r['rva']],0x19930520,1,pe.base+r['rva'],0,0,0,0)
 elif r['rva'] in data:
  assert (r['size'],r['sha256'])==data[r['rva']];assert sha(pe.read(r['rva'],r['size']))==r['sha256']
 elif r['rva'] in zeros:
  assert r['size']==zeros[r['rva']] and r['kind']=='zero' and not r['fixups'];assert initial_bytes(pe,r['rva'],r['size'])==bytes(r['size'])
 elif r['rva']==0xed728:
  assert r['size']==12 and r['anchor']=='??_7CVehicle@@6B@' and r['anchor_offset']==0
  assert pe.read(0xed728,12)==struct.pack('<III',pe.base+0xb8b50,pe.base+0xb8a10,pe.base+0xb8a40)
 else:raise AssertionError(('Unreviewed new data',r))
 for ss in o.names.values():
  for sy in ss:
   if sy['section']==r['section'] and r['offset']<=sy['value']<r['offset']+r['size']:
    address=pe.base+r['rva']+sy['value']-r['offset'];local[(r['unit'],sy['name'])]=address
    if sy['storage']==2:ids[sy['name']]=address
 ids[r['anchor']]=pe.base+r['rva']+r['anchor_offset']
assert seen==set(codes),(seen,set(codes))
for n,e in new['externals'].items():
 assert n in old['externals'],('Unexpected external',n,e)
 p=old['externals'][n];assert {k:v for k,v in p.items() if k not in ('import','library','chunks')}=={k:v for k,v in e.items() if k not in ('import','library','chunks')}
 for k in ('import','library','chunks'):
  if k in p:e[k]=p[k]
 ids[n]=e['reference_va']
for u,a in old['weak_aliases'].items():assert new['weak_aliases'][u]==a
for aliases in new['weak_aliases'].values():
 for alias,canonical in aliases.items():ids[alias]=ids[canonical]
for r in new['regions']:
 p=prior.get((r['rva'],r['size'],r['kind']))
 if p:
  assert r['sha256']==p['sha256'];assert [(f['offset'],f['kind'],f['reference_va']) for f in r['fixups']]==[(f['offset'],f['kind'],f['reference_va']) for f in p['fixups']];r['implementation']=p['implementation'];continue
 for f in r['fixups']:assert local.get((r['unit'],f['symbol']),ids.get(f['symbol']))==f['reference_va'],(r['anchor'],f)
 r['implementation']='Complete normal C++ reconstructed source with existing native ABI bridges and compiler EH; true full CVehicle layout and vtable lifetime' if r['kind']=='code' else 'Complete actual source string/float/script descriptor/global object or unwind metadata'
assert len(new['pending'])==5
for r in new['pending']:
 p=next(p for p in old['pending'] if p['anchor']==r['anchor']);r['reference_function_code_size']=p['reference_function_code_size']
 if 'limitation' in p:r['limitation']=p['limitation']
new['status']='REVIEWED';new['sdk']=json.load(open(run/'link.json',encoding='utf-8-sig'))['sdk'];assert all(new['sdk'][n]==h for n,h in old['sdk'].items())
new['review']=dict(old['review']);new['review'].update(scope='Actual vehicle constructor/lifetime2180 plus vehiclepreview707, StatisticsToString2338 and Entity native queries278.',adaptations='Normal C++ reconstruction; real CVehicle B8 layout/vtable, actual pool getters separated for original call boundaries. Statistics levels1/2/4 and true differential counters reconstructed after original source mismatch. Native query wrappers use actual RwMatrixQueryRotate/RpClumpGetNumAtomics, three real whole stores.',verification='All original function bytes, compiler EH and complete data/relocations, no masks or dependency credit. Full controls and actor ABI required. Shared header changed so both compiler profiles refreshed.',reference_credit='Hybrid vehicle constructor/destructor use missing025 lifecycle source but substantial R5 changes; direct credit restricted after whole-body attribution. Statistics and Entity queries are vendor/R5 reconstruction.',initialization_limitation='Actual CVehicle lifetime and train/carriage paths retained, no fake allocation or virtual target. No GTA/network/native execution. Actual RenderVehicle686 includes true CVehicle lifetime and whole EH21.',vehicle_vtable='ED728/12 = actual scalar B8B50, AddB8A10, RemoveB8A40 with destructorB88F0 fully implemented.')
new['review']['linker_alias_provider_evidence']=json.load(open('build/cp32-vehicle-statistics-linked1-aliases.json'))
p=Path('config/checkpoint32/vehicle-statistics-contract.json');p.write_text(json.dumps(new,indent=2)+'\n')
from verify_actor_closure import Gate
g=Gate(run.name,contract_path=str(p));g.bind();new['review']['associative_comdat_bindings']={'run':run.name,'bindings':g.associative_bindings,'negative_controls':json.load(open('build/associative-binding-controls.json')),'scope':'Existing strict associative binding, all real bytes/targets checked and no duplicate credit.'};p.write_text(json.dumps(new,indent=2)+'\n')
print('Reviewed',count,'new complete regions')
