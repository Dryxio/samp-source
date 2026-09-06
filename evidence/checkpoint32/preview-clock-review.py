import json,sys,struct
from pathlib import Path
sys.path.insert(0,'tools')
from binary import COFF,PE
from verify_checkpoint2 import import_slots
old=json.load(open('config/checkpoint32/ped-chain-contract.json'));new=json.load(open('build/actor-contract-draft.json'));prior={(r['rva'],r['size'],r['kind']):r for r in old['regions']}
pe=PE('private/samp.dll');run=Path('build/cp32-preview-clock-linked1');objs={p.stem:COFF(p) for p in run.glob('*.obj')}
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
seeds=json.load(open('config/checkpoint32/preview-clock-seeds.json'))
spec={'closure_modelinfo':[(0xb45a0,40)],'closure_remote_player_construction':[(0x165e0,122),(0x15fa0,244),(0x141b0,160)],'closure_netgame_time':[(0x88e0,5)],'closure_raknet_time':[(0x2e7f0,86),(0x2e850,155)],'closure_rpc_player_colors':[(0x19800,233),(0x3ed0,33),(0x16150,21)],'closure_preview_ped_model':[(0xaff50,113)]}
codes={}
for u,entries in spec.items():
 for rva,size in entries:
  name=next(n for n,v in seeds[u].items() if v==rva);codes[rva]=(size,name);ids[name]=pe.base+rva
codes[0xe0bf0]=(32,'__ehhandler$'+codes[0x141b0][1])
codes[0xe0f10]=(21,'__ehhandler$'+codes[0x19800][1])
handlers={0xf8918:0xe0f10};strings={}
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
 elif r['rva']==0xf858c:
  assert r['size']==44 and r['anchor_offset']==16
  assert pe.read(r['rva'],44)==struct.pack('<IIIIIIIIIII',0xffffffff,pe.base+0xe0bf0,0,pe.base+0xe0bfb,0x19930520,2,pe.base+0xf858c,0,0,0,0)
 elif r['rva']==0x118b88:
  assert r['size']==9 and r['anchor']=='_initialized' and r['anchor_offset']==8 and pe.read(r['rva'],9)==bytes(9)
  assert any(x['section']==r['section'] and x['value']==0 for x in o.names['_yo'])
  assert any(x['section']==r['section'] and x['value']==8 for x in o.names['_initialized'])
 else:raise AssertionError(('Unreviewed new data',r))
 for ss in o.names.values():
  for sy in ss:
   if sy['section']==r['section'] and r['offset']<=sy['value']<r['offset']+r['size']:
    address=pe.base+r['rva']+sy['value']-r['offset'];local[(r['unit'],sy['name'])]=address
    if sy['storage']==2:ids[sy['name']]=address
 ids[r['anchor']]=pe.base+r['rva']+r['anchor_offset']
assert seen==set(codes) and count==16,(count,seen)
for n,e in new['externals'].items():
 if n in old['externals']:
  p=old['externals'][n];assert {k:v for k,v in p.items() if k not in ('import','library','chunks')}=={k:v for k,v in e.items() if k not in ('import','library','chunks')}
  if 'chunks' in e:assert e['chunks']==p['chunks']
  for k in ('import','library','chunks'):
   if k in p:e[k]=p[k]
 else:
  from verify_checkpoint2 import import_slots,map_symbols
  if n.startswith('__imp_'):
   expected={'__imp__QueryPerformanceFrequency@4':(0x100e525c,('kernel32.dll','QueryPerformanceFrequency')),'__imp__QueryPerformanceCounter@4':(0x100e5258,('kernel32.dll','QueryPerformanceCounter'))}
   va,identity=expected[n];assert e==dict(kind='import',reference_va=va) and tuple(import_slots(pe)[va])==identity;e['import']=list(identity)
  else:
   expected={'__allmul':(0xc88b0,52,'libcmt:llmul.obj'),'__alldvrm':(0xc88f0,223,'libcmt:lldvrm.obj')}
   rva,size,owner=expected[n];assert e['kind']=='crt' and e['reference_va']==pe.base+rva and e['size']==size and e['sha256']==sha(pe.read(rva,size))
   maps=map_symbols(run/'closure.map');entries=maps[n];assert len(entries)==1 and entries[0][1].lower()==owner,(n,entries)
   linked=PE(run/'closure.dll');assert linked.read(entries[0][0]-linked.base,size)==pe.read(rva,size),'CRT whole provider bytes differ'
   e['library']='libcmt.lib'

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
new['review']=dict(old['review']);new['review'].update(scope='Real remote/net constructors526 plus EH32, RakNet clock241+NetGame time5, colors287+EH21, ped model setter113, ModelInfoLoaded40.',adaptations='Constructors, clock and colors compile exact in first C++ trial. Actual shared clock static8+1; no simulated time. ModelInfoLoaded40 uses narrow three-MOV native-field bridge after C++16/volatile40 selected wrong temporary register. Ped setter113 sourced025 with actual R5 offsets and original native ABI bridge.',verification='Whole source functions, Net ctor EH32/map44 and colors EH21/map36, true static states and all actual targets/relocations. CRT allmul52 and alldvrm223 full linked bodies equal originals with exact pinned member owners, zero code credit. Ten standard controls and actor ABI required. Seven pending remain, object preview excluded.',reference_credit='ColorsRPC233 and ped setter113 sourced025; all other new source already base/vendor or R5 reconstruction. Direct025 increment346.',model='ModelInfoLoadedB45A0 now matches the whole original40 bytes, exact native object-field1C return, not Boolean normalization. Old compiled16 and volatile40 failures excluded. CEntity SetModelIndex retains actual provider.',initialization_limitation='Actual RemotePlayer1FD allocation in NetPlayer ctor, complete original class layouts, real string unwind. No complete playerpool/spawn or outerpreview claim. Clock/native/player paths not executed.')
for r in new['regions']:
 if (r['rva'],r['size'],r['kind']) not in prior:
  r['implementation']='Complete normal C++ source and compiler EH' if r['kind']=='code' else 'Complete original clock static state or unwind metadata'
  if r['rva']==0xb45a0:r['implementation']='Complete C++ with explicitly bounded three-MOV symbolic native-field bridge; two C++ failures preserved'
  if r['rva']==0xaff50:r['implementation']='Complete025 C++ adapted to actual R5 offsets, original native audio bridge retained'
new['review']['linker_alias_provider_evidence']=json.load(open('build/cp32-preview-clock-linked1-aliases.json'))
Path('config/checkpoint32/preview-clock-contract.json').write_text(json.dumps(new,indent=2)+'\n')
from verify_actor_closure import Gate
g=Gate('cp32-preview-clock-linked1',contract_path='config/checkpoint32/preview-clock-contract.json');g.bind()
new['review']['associative_comdat_bindings']={'run':'cp32-preview-clock-linked1','bindings':g.associative_bindings,'negative_controls':json.load(open('build/associative-binding-controls.json')),'scope':'Previously tested strict associative-owner binding; all actual bytes and relocations still verified. Duplicate parent/EH gives no additional coverage.'}
Path('config/checkpoint32/preview-clock-contract.json').write_text(json.dumps(new,indent=2)+'\n')
