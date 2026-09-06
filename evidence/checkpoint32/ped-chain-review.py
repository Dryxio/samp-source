import json,sys,struct
from pathlib import Path
sys.path.insert(0,'tools')
from binary import COFF,PE
from verify_checkpoint2 import import_slots
old=json.load(open('config/checkpoint32/directory-pool-contract.json'));new=json.load(open('build/actor-contract-draft.json'));prior={(r['rva'],r['size'],r['kind']):r for r in old['regions']}
pe=PE('private/samp.dll');run=Path('build/cp32-ped-chain-linked1');objs={p.stem:COFF(p) for p in run.glob('*.obj')}
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
seeds=json.load(open('config/checkpoint32/ped-chain-seeds.json'))
spec={'closure_custom_ped_model':[(0xa7a80,76),(0xa7bd0,81)],'closure_ped_clump_stream':[(0xb1e80,65),(0xb1ed0,53),(0xb1f10,137)],'closure_ped_model_clump_setter':[(0xb45d0,36)],'closure_custom_ped_load_manager':[(0xc650,281),(0xd2c0,94)],'closure_model_instance_factory':[(0xb6250,81)],'closure_remote_player_destruction':[(0x13f60,112),(0x16660,79),(0xa0210,49),(0xa0ec0,21)]}
codes={}
for u,entries in spec.items():
 for rva,size in entries:
  name=next(n for n,v in seeds[u].items() if v==rva);codes[rva]=(size,name);ids[name]=pe.base+rva
codes[0xe0b90]=(21,'__ehhandler$'+codes[0x13f60][1])
handlers={0xf8518:0xe0b90};strings={}
literals={}
from binary import sha
local={};seen=set();count=0
for r in new['regions']:
 if (r['rva'],r['size'],r['kind']) in prior:continue
 count+=1;assert r['unit'] in spec
 o=objs[r['unit']];sec=o.sections[r['section']-1];assert sec['size']==r['size'] or (r['kind']=='script-command' and r['size']==18 and not sec['reloc_count'])
 if r['kind']=='code':
  size,name=codes[r['rva']];assert r['size']==size and r['anchor'].startswith(name);seen.add(r['rva'])
 elif r['rva'] in strings:assert (r['size'],sha(pe.read(r['rva'],r['size'])))==strings[r['rva']]
 elif r['rva'] in handlers:
  assert r['size']==36 and r['anchor_offset']==8
  assert pe.read(r['rva'],36)==struct.pack('<IIIIIIIII',0xffffffff,pe.base+handlers[r['rva']],0x19930520,1,pe.base+r['rva'],0,0,0,0)
 elif r['rva'] in literals:
  size,expected=literals[r['rva']];assert r['size']==size and pe.read(r['rva'],size)==expected.ljust(size,b'\0')
 elif r['rva']==0xec5bc:
  assert r['kind']=='script-command' and r['size']==18 and r['anchor']=='_disable_marker'
  assert sha(pe.read(r['rva'],18))=='1ebe2aef5e9dcbef37a2e24c73e67590a006311112c6df9545b27483968481ad'
 else:raise AssertionError(('Unreviewed new data',r))
 for ss in o.names.values():
  for sy in ss:
   if sy['section']==r['section'] and r['offset']<=sy['value']<r['offset']+r['size']:
    address=pe.base+r['rva']+sy['value']-r['offset'];local[(r['unit'],sy['name'])]=address
    if sy['storage']==2:ids[sy['name']]=address
 ids[r['anchor']]=pe.base+r['rva']+r['anchor_offset']
assert seen==set(codes) and count==15,(count,seen)
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
assert len(new['pending'])==8
for r in new['pending']:
 p=next(p for p in old['pending'] if p['anchor']==r['anchor']);r['reference_function_code_size']=p['reference_function_code_size']
 if 'limitation' in p:r['limitation']=p['limitation']
new['status']='REVIEWED';new['sdk']=json.load(open(run/'link.json',encoding='utf-8-sig'))['sdk'];assert all(new['sdk'][n]==h for n,h in old['sdk'].items())
new['review']=dict(old['review']);new['review'].update(scope='Type1 custom ped chain823, native model factory81, real player destruction282.',adaptations='GetModelInfo sole definition moved to custom ped TU to expose actual EDX preservation for Clone76; no duplicate provider. Ensure94 uses explicit inline getter guard. Two genuine nonvirtual method declarations added to existing complete headers. Factory81 symbolic after two bounded C++ attempts; resolver281 adapted from accepted object path after its documented failures, with actual distinct ped installer.',verification='Whole functions/data, actual source/link targets and all COFF/PE relocations. NetPlayer EH21 and complete36-byte metadata, full18-byte marker command. Ten controls and actor ABI required. Eight pending excluded; new game functions never executed.',reference_credit='Remote destructor79 adapted from025 teardown skeleton; other new bodies R5 reconstruction or already available base, direct025 increment79.',initialization_limitation='No complete preview renderer or PoolDelete/Spawn closure claim; native model virtual methods remain actual game engine boundary, outside reconstructed code coverage.')
for r in new['regions']:
 if (r['rva'],r['size'],r['kind']) not in prior:
  r['implementation']='Complete C++ source with established native ABI bridge as applicable; compiler EH' if r['kind']=='code' else 'Complete original SCRIPT_COMMAND or unwind metadata'
  if r['rva'] in (0xc650,0xb6250):r['implementation']='Explicit complete symbolic exception with named fields/local slots; bounded C++ failures documented, no raw instruction bytes'
new['review']['linker_alias_provider_evidence']=json.load(open('build/cp32-ped-chain-linked1-aliases.json'))
Path('config/checkpoint32/ped-chain-contract.json').write_text(json.dumps(new,indent=2)+'\n')
from verify_actor_closure import Gate
g=Gate('cp32-ped-chain-linked1',contract_path='config/checkpoint32/ped-chain-contract.json');g.bind()
new['review']['associative_comdat_bindings']={'run':'cp32-ped-chain-linked1','bindings':g.associative_bindings,'negative_controls':json.load(open('build/associative-binding-controls.json')),'scope':'Previously tested strict associative-owner binding; all actual bytes and relocations still verified. Duplicate parent/EH gives no additional coverage.'}
Path('config/checkpoint32/ped-chain-contract.json').write_text(json.dumps(new,indent=2)+'\n')
