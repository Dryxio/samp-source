import json,sys,struct
from pathlib import Path
sys.path.insert(0,'tools')
from binary import COFF,PE
from verify_actor_closure import initial_bytes
from verify_checkpoint2 import import_slots
old=json.load(open('config/checkpoint32/preview-clock-contract.json'));new=json.load(open('build/actor-contract-draft.json'));prior={(r['rva'],r['size'],r['kind']):r for r in old['regions']}
pe=PE('private/samp.dll');run=Path('build/cp32-map-trailer-linked2');objs={p.stem:COFF(p) for p in run.glob('*.obj')}
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
seeds=json.load(open('config/checkpoint32/map-trailer-seeds.json'))
spec={'closure_rpc_trailers':[(0x1ae50,315),(0x1af90,219)],'closure_vehicle_trailer_methods':[(0xb7c10,29),(0xb7c30,78),(0xb7c80,10)],'closure_rpc_map_icons':[(0xa300,93),(0x8fb0,45),(0xa2c0,60),(0x1a790,287),(0x1a8b0,147)],'closure_rpc_object_pool_delete':[(0x127a0,85),(0x12850,117),(0x1bc20,232)],'closure_preview_matrix_rotate':[(0xb5790,72)],'closure_preview_ped_animation':[(0xb1da0,24),(0xae530,41)]}
codes={}
for u,entries in spec.items():
 for rva,size in entries:
  name=next(n for n,v in seeds[u].items() if v==rva);codes[rva]=(size,name);ids[name]=pe.base+rva
owners=json.load(open('build/agent-independent/map-trailer-object-original-owners-preflight.json'))
handlers={};strings={};literals={}
parents={'MapSet':0x1a790,'MapDisable':0x1a8b0,'TrailerAttach':0x1ae50,'TrailerDetach':0x1af90,'ObjectDelete':0x1bc20}
for item in owners['eh_maps']:
 codes[item['whole_eh_rva']]=(21,'__ehhandler$'+codes[parents[item['function']]][1]);handlers[item['unwind_rva']]=item['whole_eh_rva']
for item in owners['descriptor_owners']:
 strings[item['rva']]=(18,item['sha256'])
from binary import sha
local={};seen=set();count=0
for r in new['regions']:
 if (r['rva'],r['size'],r['kind']) in prior:continue
 count+=1;assert r['unit'] in spec or r['unit'] in ('closure_store_preview_matrix','closure_store_trailer_attach_command','closure_store_trailer_detach_command','closure_store_trailer_detach_flag')
 o=objs[r['unit']];sec=o.sections[r['section']-1];assert sec['size']==r['size'] or (r['kind']=='script-command' and r['size']==18 and not sec['reloc_count'])
 if r['kind']=='code':
  size,name=codes[r['rva']];assert r['size']==size and r['anchor'].startswith(name);seen.add(r['rva'])
 elif r['rva'] in strings:assert (r['size'],sha(pe.read(r['rva'],r['size'])))==strings[r['rva']]
 elif r['rva'] in handlers:
  assert r['size']==36 and r['anchor_offset']==8
  assert pe.read(r['rva'],36)==struct.pack('<IIIIIIIII',0xffffffff,pe.base+handlers[r['rva']],0x19930520,1,pe.base+r['rva'],0,0,0,0)
 elif r['rva'] in literals:
  size,expected=literals[r['rva']];assert r['size']==size and pe.read(r['rva'],size)==expected.ljust(size,b'\0')
 elif r['rva']==0x151744:
  assert r['size']==4 and r['kind']=='zero' and r['anchor']=='?r5TrailerDetachInProgress@@3HA' and initial_bytes(pe,r['rva'],4)==bytes(4)
 elif r['rva']==0x117400:
  assert r['size']==40 and pe.read(r['rva'],40)==struct.pack('<9fI',1,0,0,0,1,0,0,0,1,0x7f1fd0)
  assert any(x['section']==r['section'] and x['value']==0 for x in o.names['?r5MatrixRotationAxes@@3PAU_VECTOR@@A'])
  assert any(x['section']==r['section'] and x['value']==36 for x in o.names['?r5RwMatrixRotateFunction@@3KA'])
 else:raise AssertionError(('Unreviewed new data',r))
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
 else:raise AssertionError(('Unexpected external',n,e))
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
assert len(new['pending'])==7
for r in new['pending']:
 p=next(p for p in old['pending'] if p['anchor']==r['anchor']);r['reference_function_code_size']=p['reference_function_code_size']
 if 'limitation' in p:r['limitation']=p['limitation']
new['status']='REVIEWED';new['sdk']=json.load(open(run/'link.json',encoding='utf-8-sig'))['sdk'];assert all(new['sdk'][n]==h for n,h in old['sdk'].items())
new['review']=dict(old['review']);new['review'].update(scope='TrailerRPC/methods693, map icons674, object pool deletion455, matrix/animation137.',adaptations='True pool accessors restore ordinary C++ load order. Map helper statements expanded where original inlines them. Trailer methods moved into sole separate TU to preserve actual caller clobber assumptions; no wrappers. CVehicle gap48 identified as real trailer pointer, size unchanged, three nonvirtual methods declared. Matrix axis calculation precedes function-pointer assignment; native ABI idioms retained.',verification='Whole bodies, five EH21/maps36, actual flag151744/4, complete script descriptors and matrix data40, all actual providers and relocations. Distinct map markerE5958 and game markerEC5BC never aliased. Standard ten controls and actor ABI required; seven old pending excluded.',reference_credit='025 contributes trailer principal651, mapRPC434, objectRPC232+poolDelete117, total1434 adapted sourcebytes. EH/native/newmodel helpers excluded from this attribution.',initialization_limitation='Trailer flag true module state with writes in real DetachTrailer and reader in original A4C70 hook; hook not covered. Pool views never allocated, real CObject virtual deleting destructor slot0/flags1 from accepted lifetime. Native render/animation not executed; whole preview still excluded.')
for r in new['regions']:
 if (r['rva'],r['size'],r['kind']) not in prior:r['implementation']='Complete normal C++ and original native ABI bridges as applicable; compiler EH' if r['kind']=='code' else 'Complete true script command, matrix axes/function pointer, module flag or EH metadata'
new['review']['linker_alias_provider_evidence']=json.load(open('build/cp32-map-trailer-linked2-aliases.json'))
Path('config/checkpoint32/map-trailer-contract.json').write_text(json.dumps(new,indent=2)+'\n')
from verify_actor_closure import Gate
g=Gate('cp32-map-trailer-linked2',contract_path='config/checkpoint32/map-trailer-contract.json');g.bind()
new['review']['associative_comdat_bindings']={'run':'cp32-map-trailer-linked2','bindings':g.associative_bindings,'negative_controls':json.load(open('build/associative-binding-controls.json')),'scope':'Previously tested strict associative-owner binding; all actual bytes and relocations still verified. Duplicate parent/EH gives no additional coverage.'}
Path('config/checkpoint32/map-trailer-contract.json').write_text(json.dumps(new,indent=2)+'\n')
