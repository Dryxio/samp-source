import json,sys,struct
from pathlib import Path
sys.path.insert(0,'tools')
from binary import COFF,PE
from verify_checkpoint2 import import_slots
old=json.load(open('config/checkpoint32/textdraw-init-contract.json'));new=json.load(open('build/actor-contract-draft.json'));prior={(r['rva'],r['size'],r['kind']):r for r in old['regions']}
pe=PE('private/samp.dll');run=Path('build/cp32-textdraw-vehicle-rpc-linked1');objs={p.stem:COFF(p) for p in run.glob('*.obj')}
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
seeds=json.load(open('config/checkpoint32/textdraw-vehicle-rpc-seeds.json'))
spec={'closure_rpc_edit_textdraw':[(0x1cf70,268)],'closure_rpc_show_textdraw':[(0x1cd90,297)],'closure_chat_info':[(0x68070,117)],'closure_vehicle_appearance_interior':[(0xb8150,25),(0xb7940,232),(0xad340,150),(0x1c230,280),(0x18b70,218),(0x19a00,156)]}
codes={}
for u,entries in spec.items():
 for rva,size in entries:
  name=next(n for n,v in seeds[u].items() if v==rva);codes[rva]=(size,name);ids[name]=pe.base+rva
handlers={0xf8ffc:0xe1530,0xf8fb4:0xe14f0,0xf8e4c:0xe13b0,0xf8744:0xe0d70,0xf8960:0xe0f50}
for rva,parent in [(0xe1530,0x1cf70),(0xe14f0,0x1cd90),(0xe13b0,0x1c230),(0xe0d70,0x18b70),(0xe0f50,0x19a00)]:codes[rva]=(21,'__ehhandler$'+codes[parent][1])
strings={0xe6404:(41,'7cd74165e52bf9c4ea3d65d5ecc3036240b1ce9d26488c2a60138f92d17e6000')}
from binary import sha
from verify_checkpoint2 import map_symbols
maps=map_symbols(run/'closure.map')
assert pe.read(0xe5cc8,3)==b'%X\0'
ids['??_C@_02EMFKHFLK@?$CFX?$AA@']=pe.base+0xe5cc8
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
 else:
  commands={0xecdd0:(0x4bb,b'i'),0xecfec:(0x860,b'ii'),0xecf4c:(0x4e4,b'ff')}
  op,args=commands[r['rva']];assert r['kind']=='script-command' and r['size']==18 and pe.read(r['rva'],18)==struct.pack('<H',op)+args.ljust(16,b'\0')
 for ss in o.names.values():
  for sy in ss:
   if sy['section']==r['section'] and r['offset']<=sy['value']<r['offset']+r['size']:
    address=pe.base+r['rva']+sy['value']-r['offset'];local[(r['unit'],sy['name'])]=address
    if sy['storage']==2:ids[sy['name']]=address
 ids[r['anchor']]=pe.base+r['rva']+r['anchor_offset']
assert seen==set(codes) and count==23,(count,seen)
for n,e in new['externals'].items():
 if n in old['externals']:
  p=old['externals'][n];assert {k:v for k,v in p.items() if k not in ('import','library','chunks')}=={k:v for k,v in e.items() if k not in ('import','library','chunks')}
  if 'chunks' in e:assert e['chunks']==p['chunks']
  for k in ('import','library','chunks'):
   if k in p:e[k]=p[k]
 else:raise AssertionError(('Unexpected new external',n,e))
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
new['review']=dict(old['review']);new['review'].update(scope='Show/Edit textdraw RPCs and actual Info helper724, vehicle appearance/interior1124.',adaptations='Edit length initialization precedes pool extraction; number plate length initialization precedes text buffer memset33. These two motivated ordering fixes close complete candidate mismatches. Show297 andInfo117 exact first trial, true infoColor126/type4 rather than debugColor12A/type8. Vehicle partial views are pointer-only and native wheel layouts asserted; no new allocation/global owners.',verification='All nine whole principals plus five21-byte EH and five36-byte maps, three complete SCRIPT_COMMAND18 objects and warning literal41. All previous providers/pending retained, every actual target reviewed. Full gate, ten standard controls and actor ABI required.',reference_credit='524 direct adapted025 bytes: TireRPC218 strongly adapted to R5 one-byte mask; Interior150+RPC156. Textdraw changed protocol/Info reconstructed from existing base:0 strict direct credit. Plate305 and Tire provider232 R5-only.')
new['review']['linker_alias_provider_evidence']=json.load(open('build/cp32-textdraw-vehicle-rpc-linked1-aliases.json'))
Path('config/checkpoint32/textdraw-vehicle-rpc-contract.json').write_text(json.dumps(new,indent=2)+'\n')
from verify_actor_closure import Gate
g=Gate('cp32-textdraw-vehicle-rpc-linked1',contract_path='config/checkpoint32/textdraw-vehicle-rpc-contract.json');g.bind()
new['review']['associative_comdat_bindings']={'run':'cp32-textdraw-vehicle-rpc-linked1','bindings':g.associative_bindings,'negative_controls':json.load(open('build/associative-binding-controls.json')),'scope':'Previously tested strict associative-owner binding; all actual bytes and relocations still verified. Duplicate parent/EH gives no additional coverage.'}
Path('config/checkpoint32/textdraw-vehicle-rpc-contract.json').write_text(json.dumps(new,indent=2)+'\n')
