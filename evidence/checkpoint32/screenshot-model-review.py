import json,sys,struct
from pathlib import Path
sys.path.insert(0,'tools')
from binary import COFF,PE
from verify_checkpoint2 import import_slots
old=json.load(open('config/checkpoint32/name-distance-contract.json'));new=json.load(open('build/actor-contract-draft.json'));prior={(r['rva'],r['size'],r['kind']):r for r in old['regions']}
pe=PE('private/samp.dll');run=Path('build/cp32-screenshot-model-linked1');objs={p.stem:COFF(p) for p in run.glob('*.obj')}
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
seeds=json.load(open('config/checkpoint32/screenshot-model-seeds.json'))
spec={'closure_screenshot_filename':[(0x62270,255),(0xc3ad0,6)],'closure_scoreboard_foundations':[(0x6ea30,125),(0x6e930,84)],'closure_model_preview_utilities':[(0xb6360,74),(0xb6410,93),(0xb4520,9),(0xb1430,20)],'closure_rw_model_dispatch':[(0xb62f0,59),(0xb6330,37),(0xb62b0,57)]}
codes={}
for u,entries in spec.items():
 for rva,size in entries:
  name=next(n for n,v in seeds[u].items() if v==rva);codes[rva]=(size,name);ids[name]=pe.base+rva
stringbase='?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@'
plus='??$?HDU?$char_traits@D@std@@V?$allocator@D@1@@std@@YA?AV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@0@ABV10@PBD@Z'
for rva,size,name in [(0x61f60,233,'?append@'+stringbase+'QAEAAV12@ABV12@II@Z'),(0x62050,271,'?append@'+stringbase+'QAEAAV12@PBDI@Z'),(0x621c0,167,plus),(0x13f30,38,'??1'+stringbase+'QAE@XZ')]:codes[rva]=(size,name);ids[name]=pe.base+rva
codes[0xe3780]=(18,'__ehhandler$'+plus)
handlers={}
strings={}
from binary import sha
from verify_checkpoint2 import map_symbols
maps=map_symbols(run/'closure.map')
assert pe.read(0xe5cc8,3)==b'%X\0'
ids['??_C@_02EMFKHFLK@?$CFX?$AA@']=pe.base+0xe5cc8
local={};seen=set();count=0
for r in new['regions']:
 if (r['rva'],r['size'],r['kind']) in prior:continue
 count+=1;assert r['unit'] in spec or r['unit']=='closure_store_samp_directory'
 o=objs[r['unit']];sec=o.sections[r['section']-1];assert sec['size']==r['size'] or (r['kind']=='script-command' and r['size']==18 and not sec['reloc_count'])
 if r['kind']=='code':
  size,name=codes[r['rva']];assert r['size']==size and r['anchor'].startswith(name);seen.add(r['rva'])
 elif r['rva'] in strings:assert (r['size'],sha(pe.read(r['rva'],r['size'])))==strings[r['rva']]
 elif r['rva'] in handlers:
  assert r['size']==36 and r['anchor_offset']==8
  assert pe.read(r['rva'],36)==struct.pack('<IIIIIIIII',0xffffffff,pe.base+handlers[r['rva']],0x19930520,1,pe.base+r['rva'],0,0,0,0)
 elif r['rva']==0xe98f0:
  assert r['size']==24 and pe.read(r['rva'],24)==b'\\screens\\sa-mp-%03i.png\0'.ljust(24,b'\0')
 elif r['rva']==0x26e918:
  assert r['size']==261 and r['kind']=='zero' and r['anchor']=='?szSAMPDir@@3PADA'
 elif r['rva']==0xfb5d8:
  assert r['size']==36 and r['anchor_offset']==8
  assert pe.read(r['rva'],36)==struct.pack('<IIIIIIIII',0xffffffff,pe.base+0xe3780,0x19930520,1,pe.base+0xfb5d8,0,0,0,0)
 else:raise AssertionError(('Unreviewed new data',r))
 for ss in o.names.values():
  for sy in ss:
   if sy['section']==r['section'] and r['offset']<=sy['value']<r['offset']+r['size']:
    address=pe.base+r['rva']+sy['value']-r['offset'];local[(r['unit'],sy['name'])]=address
    if sy['storage']==2:ids[sy['name']]=address
 ids[r['anchor']]=pe.base+r['rva']+r['anchor_offset']
assert seen==set(codes) and count==19,(count,seen)
for n,e in new['externals'].items():
 if n in old['externals']:
  p=old['externals'][n];assert {k:v for k,v in p.items() if k not in ('import','library','chunks')}=={k:v for k,v in e.items() if k not in ('import','library','chunks')}
  if 'chunks' in e:assert e['chunks']==p['chunks']
  for k in ('import','library','chunks'):
   if k in p:e[k]=p[k]
 else:
  expected={'__imp__FindFirstFileA@8':(0x100e524c,('kernel32.dll','FindFirstFileA')),'__imp__FindClose@4':(0x100e5244,('kernel32.dll','FindClose'))}
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
new['review']=dict(old['review']);new['review'].update(scope='Screenshot filename/std::string closure988, ScoreBoard209 and model utilities349.',adaptations='Screenshot255 and actual MSVC append/operator+/destructor source exact first trial, original format24/stack260/global261 sizes retained. ScoreBoard209 resurrected from historical candidates with all constants checked. Collision getters two C++ trials isolate EDX transfer; final three named MOVs per getter preserve original stack locals. RenderAtomic59 explicitly symbolic ABI bridge after C++41 mismatch. Other utility functions remain C++.',verification='Whole functions, complete EH18/map36, literal24 and real261-byte global owner. Every actual source/link target and relocation verified; no new runtime execution. Eight old pending excluded. Full gate, ten standard negative controls and actor ABI required.',reference_credit='255 direct025 adapted screenshot filename function. Other code derives from existing base, pinned Microsoft header templates or R5 reconstruction; direct025zero.',initialization_limitation='szSAMPDir is real initially-zero261-byte object. Its actual SetupDirectoriesC3C70 writer is identified but not implemented or credited in this lot. No directory creation/screenshot operation was executed.')
for r in new['regions']:
 if (r['rva'],r['size'],r['kind']) not in prior and r['kind']=='code':
  r['implementation']='Explicit symbolic ABI bridge after normal C++ mismatch' if r['rva']==0xb62f0 else 'C++ with three named symbolic MOVs after two documented normal C++ mismatches' if r['rva'] in (0xb6360,0xb6410) else 'Complete C++ source or compiler-generated EH'
new['review']['linker_alias_provider_evidence']=json.load(open('build/cp32-screenshot-model-linked1-aliases.json'))
Path('config/checkpoint32/screenshot-model-contract.json').write_text(json.dumps(new,indent=2)+'\n')
from verify_actor_closure import Gate
g=Gate('cp32-screenshot-model-linked1',contract_path='config/checkpoint32/screenshot-model-contract.json');g.bind()
new['review']['associative_comdat_bindings']={'run':'cp32-screenshot-model-linked1','bindings':g.associative_bindings,'negative_controls':json.load(open('build/associative-binding-controls.json')),'scope':'Previously tested strict associative-owner binding; all actual bytes and relocations still verified. Duplicate parent/EH gives no additional coverage.'}
Path('config/checkpoint32/screenshot-model-contract.json').write_text(json.dumps(new,indent=2)+'\n')
