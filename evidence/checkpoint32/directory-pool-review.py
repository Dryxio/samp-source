import json,sys,struct
from pathlib import Path
sys.path.insert(0,'tools')
from binary import COFF,PE
from verify_checkpoint2 import import_slots
old=json.load(open('config/checkpoint32/screenshot-model-contract.json'));new=json.load(open('build/actor-contract-draft.json'));prior={(r['rva'],r['size'],r['kind']):r for r in old['regions']}
pe=PE('private/samp.dll');run=Path('build/cp32-directory-pool-linked1');objs={p.stem:COFF(p) for p in run.glob('*.obj')}
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
seeds=json.load(open('config/checkpoint32/directory-pool-seeds.json'))
spec={'closure_setup_directories':[(0xc3af0,258),(0xc3c70,180)],'closure_local_player_construction':[(0x4c50,223),(0x2e90,263)],'closure_player_pool_construction':[(0x13fd0,154)]}
codes={}
for u,entries in spec.items():
 for rva,size in entries:
  name=next(n for n,v in seeds[u].items() if v==rva);codes[rva]=(size,name);ids[name]=pe.base+rva
codes[0xe0bb0]=(32,'__ehhandler$'+codes[0x13fd0][1])
handlers={};strings={}
literals={r['rva']:(r['size'],bytes.fromhex(r['hex'])) for r in json.load(open('build/agent-independent/setup-directories-audit.json'))['literals']}
from binary import sha
from verify_checkpoint2 import map_symbols
maps=map_symbols(run/'closure.map')
assert pe.read(0xe5cc8,3)==b'%X\0'
ids['??_C@_02EMFKHFLK@?$CFX?$AA@']=pe.base+0xe5cc8
local={};seen=set();count=0
for r in new['regions']:
 if (r['rva'],r['size'],r['kind']) in prior:continue
 count+=1;assert r['unit'] in spec or r['unit']=='closure_store_cache_directory'
 o=objs[r['unit']];sec=o.sections[r['section']-1];assert sec['size']==r['size'] or (r['kind']=='script-command' and r['size']==18 and not sec['reloc_count'])
 if r['kind']=='code':
  size,name=codes[r['rva']];assert r['size']==size and r['anchor'].startswith(name);seen.add(r['rva'])
 elif r['rva'] in strings:assert (r['size'],sha(pe.read(r['rva'],r['size'])))==strings[r['rva']]
 elif r['rva'] in handlers:
  assert r['size']==36 and r['anchor_offset']==8
  assert pe.read(r['rva'],36)==struct.pack('<IIIIIIIII',0xffffffff,pe.base+handlers[r['rva']],0x19930520,1,pe.base+r['rva'],0,0,0,0)
 elif r['rva'] in literals:
  size,expected=literals[r['rva']];assert r['size']==size and pe.read(r['rva'],size)==expected.ljust(size,b'\0')
 elif r['rva']==0x26ea20:
  assert r['size']==261 and r['kind']=='zero' and r['anchor']=='?szCacheDir@@3PADA'
 elif r['rva']==0xf853c:
  assert r['size']==44 and r['anchor_offset']==16
  assert pe.read(r['rva'],44)==struct.pack('<IIIIIIIIIII',0xffffffff,pe.base+0xe0bb0,0,pe.base+0xe0bbb,0x19930520,2,pe.base+0xf853c,0,0,0,0)
 else:raise AssertionError(('Unreviewed new data',r))
 for ss in o.names.values():
  for sy in ss:
   if sy['section']==r['section'] and r['offset']<=sy['value']<r['offset']+r['size']:
    address=pe.base+r['rva']+sy['value']-r['offset'];local[(r['unit'],sy['name'])]=address
    if sy['storage']==2:ids[sy['name']]=address
 ids[r['anchor']]=pe.base+r['rva']+r['anchor_offset']
assert seen==set(codes) and count==14,(count,seen)
for n,e in new['externals'].items():
 if n in old['externals']:
  p=old['externals'][n];assert {k:v for k,v in p.items() if k not in ('import','library','chunks')}=={k:v for k,v in e.items() if k not in ('import','library','chunks')}
  if 'chunks' in e:assert e['chunks']==p['chunks']
  for k in ('import','library','chunks'):
   if k in p:e[k]=p[k]
 else:
  expected={'__imp__GetCurrentDirectoryA@8':(0x100e5170,('kernel32.dll','GetCurrentDirectoryA')),'__imp__CreateDirectoryA@8':(0x100e523c,('kernel32.dll','CreateDirectoryA')),'__imp__RegOpenKeyExA@20':(0x100e5004,('advapi32.dll','RegOpenKeyExA')),'__imp__RegQueryValueExA@24':(0x100e5008,('advapi32.dll','RegQueryValueExA'))}
  va,identity=expected[n];assert e==dict(kind='import',reference_va=va) and tuple(import_slots(pe)[va])==identity;e['import']=list(identity)
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
new['review']=dict(old['review']);new['review'].update(scope='Actual directory/cache initialization438 and LocalPlayer/PlayerPool construction672.',adaptations='Two full base directory functions, include game/util.h for true existing file-existence provider. LocalPlayer constructor/reset unchanged base C++; pool constructor explicitly preserves original setter-shaped upper-bound condition. Real existing headers assert LocalPlayer0x324 and PlayerPool0x2F3E, actual new allocation is0x324. No invented partial allocation or destructor closure.',verification='Every whole code/data region, EH32 plus complete44-byte map, actual directory buffers261 and six literals, all true source/link targets and relocations. Full gate, ten controls and actor ABI required; eight old pending excluded. Directory/registry/player construction functions not executed.',reference_credit='All new functions already exist in base, no incremental025 credit.',initialization_limitation='SetupDirectories and SetupCacheDirectories now implemented and exactly matched, closing real szSAMPDir/szCacheDir writers. No directory/registry operation executed. PlayerPool and LocalPlayer destruction chains remain outside new coverage.')
for r in new['regions']:
 if (r['rva'],r['size'],r['kind']) not in prior:r['implementation']='Complete original C++ source and compiler EH' if r['kind']=='code' else 'Complete source string/buffer or original unwind metadata'
new['review']['linker_alias_provider_evidence']=json.load(open('build/cp32-directory-pool-linked1-aliases.json'))
Path('config/checkpoint32/directory-pool-contract.json').write_text(json.dumps(new,indent=2)+'\n')
from verify_actor_closure import Gate
g=Gate('cp32-directory-pool-linked1',contract_path='config/checkpoint32/directory-pool-contract.json');g.bind()
new['review']['associative_comdat_bindings']={'run':'cp32-directory-pool-linked1','bindings':g.associative_bindings,'negative_controls':json.load(open('build/associative-binding-controls.json')),'scope':'Previously tested strict associative-owner binding; all actual bytes and relocations still verified. Duplicate parent/EH gives no additional coverage.'}
Path('config/checkpoint32/directory-pool-contract.json').write_text(json.dumps(new,indent=2)+'\n')
