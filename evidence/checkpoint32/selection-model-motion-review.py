import json,sys,struct
from pathlib import Path
sys.path.insert(0,'tools')
from binary import COFF,PE
from verify_checkpoint2 import import_slots
old=json.load(open('config/checkpoint32/player-lifetime-contract.json'));new=json.load(open('build/actor-contract-draft.json'));prior={(r['rva'],r['size'],r['kind']):r for r in old['regions']}
pe=PE('private/samp.dll');run=Path('build/cp32-selection-model-motion-linked3');objs={p.stem:COFF(p) for p in run.glob('*.obj')}
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
seeds=json.load(open('config/checkpoint32/selection-model-motion-seeds.json'));codes={};data_eh=set()
spec={'closure_object_editor_attached':[(0x72ac0,241)],'closure_object_editor_close':[(0x72bc0,109)],'closure_rpc_select_textdraw':[(0x1d650,247)],'closure_custom_model_load_symbolic':[(0xc770,281),(0xd320,105)],'closure_rpc_player_motion_shop':[(0x17e50,247),(0x18850,248),(0x18c50,166),(0x18d00,166),(0x18e50,174)]}
for unit,entries in spec.items():
 for rva,size in entries:codes[rva]=(size,next(n for n,v in seeds[unit].items() if v==rva))
handlers={0xf90f8:0xe1610,0xfb98c:0xe3aa0}
for rva,parent in [(0xe1610,0x1d650),(0xe3aa0,0x72ac0)]:codes[rva]=(21,'__ehhandler$'+codes[parent][1])
for row in json.load(open('build/agent-independent/rpc-player-motion-shop-audit.json'))['rows']:
 e=row['EH'];rva=int(e['cleanup'],16);codes[rva]=(21,'__ehhandler$'+codes[int(row['rva'],16)][1]);handlers[int(e['UnwindMap']['rva'],16)]=rva
for unit in spec:
 for name,rva in seeds[unit].items():ids[name]=pe.base+rva
formats={a['name']:(int(a['rva'],16),bytes(a['byte_values'])) for a in json.load(open('build/agent-textdraw/custom-model-load-symbolic-manifest.json'))['data']}
reviewed_aliases={n:a['real_symbol'] for n,a in json.load(open('build/agent-textdraw/rw-parser-symbolic-aliases.json'))['aliases'].items()}
assert new['weak_aliases']['closure_rw_parser_symbolic']==reviewed_aliases
for alias,canonical in reviewed_aliases.items():ids[alias]=ids[canonical]
for unit,aliases in old['weak_aliases'].items():assert new['weak_aliases'][unit]==aliases

local={};seen=set();count=0
for r in new['regions']:
 if (r['rva'],r['size'],r['kind']) in prior:continue
 count+=1;assert r['unit'] in spec or r['unit'].startswith('closure_custom_model_format_') or r['unit'] in ('closure_store_rpc_edit_attached','closure_store_textdraw_selector','closure_store_object_editor','closure_store_object_selection');o=objs[r['unit']];sec=o.sections[r['section']-1];assert sec['size']==r['size'] or (r['kind']=='script-command' and r['size']==18 and not sec['reloc_count'])
 if r['kind']=='code':
  size,prefix=codes[r['rva']];assert r['size']==size and r['anchor'].startswith(prefix);seen.add(r['rva'])
 else:
  commands={0xe6220:(0x52c,b'ii'),0xe6234:(0x3fd,b'ii')}
  if r['rva'] in commands:
   op,args=commands[r['rva']];assert r['size']==18 and pe.read(r['rva'],18)==struct.pack('<H',op)+args.ljust(16,b'\0')
  elif r['rva'] in (0x26eb60,0x26eb64,0x26eb68):assert r['kind']=='zero' and r['size']==4
  elif r['rva']==0xe5ea4:assert r['size']==4 and pe.read(r['rva'],4)==struct.pack('<I',116)
  elif r['rva']==0xe59c9:assert r['size']==1 and pe.read(r['rva'],1)==b'\0'
  elif r['rva'] in [v[0] for v in formats.values()]:
   expected=next(data for rva,data in formats.values() if rva==r['rva']);assert r['size']==len(expected) and pe.read(r['rva'],r['size'])==expected
  else:
   assert r['size']==36 and r['anchor_offset']==8 and r['rva'] in handlers
   assert pe.read(r['rva'],36)==struct.pack('<IIIIIIIII',0xffffffff,pe.base+handlers[r['rva']],0x19930520,1,pe.base+r['rva'],0,0,0,0)
 for ss in o.names.values():
  for sy in ss:
   if sy['section']==r['section'] and r['offset']<=sy['value']<r['offset']+r['size']:
    address=pe.base+r['rva']+sy['value']-r['offset'];local[(r['unit'],sy['name'])]=address
    if sy['storage']==2:ids[sy['name']]=address
 ids[r['anchor']]=pe.base+r['rva']+r['anchor_offset']
assert seen==set(codes) and count==len(codes)+16,(count,len(codes),set(codes)-seen) # previously covered Entity provider is now linked into this closure

from verify_checkpoint2 import map_symbols
for n,e in new['externals'].items():
 if n in old['externals']:
  assert {k:v for k,v in old['externals'][n].items() if k not in ('import','library','chunks')}=={k:v for k,v in e.items() if k not in ('import','library','chunks')}
  if 'chunks' in e:assert e['chunks']==old['externals'][n]['chunks']
  for k in ('import','library','chunks'):
   if k in old['externals'][n]:e[k]=old['externals'][n][k]
 else:
  assert n=='_fread' and e==dict(kind='crt',reference_va=0x100c8482,size=76,sha256='b846334a79bf2723effce0ff2b8731139751c38c123fc7cb28d9b4450cb55d1c')
  assert any(owner.upper()=='LIBCMT:FREAD.OBJ' for _,owner in map_symbols(run/'closure.map')[n])
 ids[n]=e['reference_va']
for r in new['regions']:
 p=prior.get((r['rva'],r['size'],r['kind']))
 if p:
  assert r['sha256']==p['sha256'];assert [(f['offset'],f['kind'],f['reference_va']) for f in r['fixups']]==[(f['offset'],f['kind'],f['reference_va']) for f in p['fixups']];r['implementation']=p['implementation'];continue
 for f in r['fixups']:assert local.get((r['unit'],f['symbol']),ids.get(f['symbol']))==f['reference_va'],(r['anchor'],f)
 r['implementation']='C++' if r['kind']=='code' else 'complete packed timer/static initialization guard/input context object' if r['kind']=='zero' else 'complete source double constant'
assert len(new['pending'])==8
for r in new['pending']:
 p=next(p for p in old['pending'] if p['anchor']==r['anchor']);r['reference_function_code_size']=p['reference_function_code_size']
 if 'limitation' in p:r['limitation']=p['limitation']
for r in new['regions']:
 if (r['rva'],r['size'],r['kind']) not in prior:r['implementation']='C++' if r['kind']=='code' else 'complete source dynamic API pointer storage' if r['kind']=='zero' else 'complete original DLL/API name string'
new['status']='REVIEWED';new['sdk']=json.load(open(run/'link.json',encoding='utf-8-sig'))['sdk']
assert all(new['sdk'][n]==h for n,h in old['sdk'].items())
new['review']=dict(old['review']);new['review'].update(scope='Complete selection/editor, custom model load closure and five player RPCs.',adaptations='Close109 uses explicit if response assignment, preserving original branch code. Shop247 uses full memset33 instead of array initializer; Armed174 uses original early-return control flow. Three025 RPCs exact first trial. C770281 is labeled symbolic exception after three failed C++ layouts; actual install provider A7C30 fixes identity rather than homologueC650. EnsureReady105 remains normal C++. Source formats live in three complete independent units with one literal backslash. Shop original provider order is SetShopName then LoadShoppingDataSubsection; a raw matching candidate with swapped names was rejected by placement review before acceptance. The empty shop string was already covered. Three interface globals were identified but missing, now full4-byte zero-fill stores with actual DoInitStuff writers audited. All partial views retain accepted provider declarations.',verification='All whole principals and seven21-byte EH handlers with36-byte maps, two complete SCRIPT_COMMAND18 objects, real attachedRPC116 and original strings. No new external provider, no native/network/UI execution. Fullgate, ten negative controls and actorABI required.',reference_credit='580 direct adapted025 bytes from Velocity248 and DrunkVisuals/Handling166 each. All other new code R5, including model loader281 symbolic exception.')
new['review']['linker_alias_provider_evidence']=json.load(open('build/cp32-selection-model-motion-linked3-aliases.json'))
for r in new['regions']:
 if (r['rva'],r['size'],r['kind']) not in prior:
  r['implementation']=('Explicit R5 symbolic exception after three failed normal C++ branch layouts' if r['rva']==0xc770 else 'C++ adapted025 whole RPC with actual R5 BitStream char* constructor' if r['rva'] in (0x18850,0x18c50,0x18d00) else 'C++ whole R5 function, complete compiler EH') if r['kind']=='code' else 'Complete source format/string, typed SCRIPT_COMMAND/RPC identifier or unwind metadata'
new['review']['associative_comdat_bindings']=dict(run='cp32-selection-model-motion-linked3',bindings=json.load(open('build/selection-model-motion-associative-bindings.json')),negative_controls=json.load(open('build/associative-binding-controls.json')),scope='Only COFF selection5 associated sections of identical accepted source parent COMDATs can defer missing local MAP anchors. Actual source parent owner is required; all source/linked bytes and relocation sets remain fully compared. No extra coverage for duplicates.')
Path('config/checkpoint32/selection-model-motion-contract.json').write_text(json.dumps(new,indent=2)+'\n')
