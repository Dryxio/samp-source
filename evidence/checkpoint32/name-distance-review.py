import json,sys,struct
from pathlib import Path
sys.path.insert(0,'tools')
from binary import COFF,PE
from verify_checkpoint2 import import_slots
old=json.load(open('config/checkpoint32/textdraw-vehicle-rpc-contract.json'));new=json.load(open('build/actor-contract-draft.json'));prior={(r['rva'],r['size'],r['kind']):r for r in old['regions']}
pe=PE('private/samp.dll');run=Path('build/cp32-name-distance-linked2');objs={p.stem:COFF(p) for p in run.glob('*.obj')}
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
seeds=json.load(open('config/checkpoint32/name-distance-seeds.json'))
spec={'closure_rpc_player_name':[(0x1dfd0,337)],'closure_entity_local_distances':[(0x9f0c0,199),(0x9f1f0,180)],'closure_preview_independent_queries':[(0xb3db0,19),(0xd110,54)]}
codes={}
for u,entries in spec.items():
 for rva,size in entries:
  name=next(n for n,v in seeds[u].items() if v==rva);codes[rva]=(size,name);ids[name]=pe.base+rva
stringbase='?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@'
for rva,size,name in [(0xb7b0,234,'?assign@'+stringbase+'QAEAAV12@PBDI@Z'),(0xb6c0,227,'?assign@'+stringbase+'QAEAAV12@ABV12@II@Z'),(0xb120,316,'?_Copy@'+stringbase+'IAEXII@Z'),(0xb0a0,117,'?erase@'+stringbase+'QAEAAV12@II@Z')]:codes[rva]=(size,name);ids[name]=pe.base+rva
handlers={0xf9218:0xe1710}
for rva,parent,size in [(0xe1710,0x1dfd0,21),(0xe04d0,0xb120,10)]:codes[rva]=(size,'__ehhandler$'+codes[parent][1])
strings={}
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
 elif r['rva']==0xec54c:
  assert r['size']==4 and pe.read(r['rva'],4)==struct.pack('<f',100000.0)
 elif r['rva']==0xf7d18:
  assert r['size']==132 and r['anchor_offset']==104
  assert sha(pe.read(r['rva'],132))=='285ecf15f9521731569675a170e30687f8c07aac93ad884efc2bd7470b0d0042'
  # Four unwind records, two catch-all HandlerTypes, two TryBlockMap entries, full FuncInfo.
  d=struct.unpack('<33I',pe.read(r['rva'],132));assert d[26]==0x19930520 and d[27]==4 and d[28]==pe.base+0xf7d18 and d[29]==2 and d[30]==pe.base+0xf7d58
 else:raise AssertionError(('Unreviewed new data',r))
 for ss in o.names.values():
  for sy in ss:
   if sy['section']==r['section'] and r['offset']<=sy['value']<r['offset']+r['size']:
    address=pe.base+r['rva']+sy['value']-r['offset'];local[(r['unit'],sy['name'])]=address
    if sy['storage']==2:ids[sy['name']]=address
 ids[r['anchor']]=pe.base+r['rva']+r['anchor_offset']
assert seen==set(codes) and count==13,(count,seen)
for n,e in new['externals'].items():
 if n in old['externals']:
  p=old['externals'][n];assert {k:v for k,v in p.items() if k not in ('import','library','chunks')}=={k:v for k,v in e.items() if k not in ('import','library','chunks')}
  if 'chunks' in e:assert e['chunks']==p['chunks']
  for k in ('import','library','chunks'):
   if k in p:e[k]=p[k]
 else:
  expected={'?_Xran@_String_base@std@@QBEXXZ':(0xc631c,63,'cabdffd07bd22757b65375ca7cb37e3612140f7183304b1f3cb873b34b93e143','LIBCPMT:STRING.OBJ'),'?_Xlen@_String_base@std@@QBEXXZ':(0xc6374,63,'be0ff18e9dc712078e091c0235989d8151577d0b299fd99cdd93a30eb62771f8','LIBCPMT:STRING.OBJ'),'__CxxThrowException@8':(0xc75c5,58,'303a80efc042dbf98623ec59b57408bb66201bc1e11e82cdb5ab41be48bf6f6c','LIBCMT:THROW.OBJ')}
  rva,size,h,owner=expected[n];assert e==dict(kind='crt',reference_va=pe.base+rva,size=size,sha256=h);assert any(provider.upper()==owner for _,provider in maps[n])
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
new['review']=dict(old['review']);new['review'].update(scope='PlayerName337 plus actual std::string closure925, Entity local distances379 and independent preview predicates73.',adaptations='PlayerName uses real existing CPlayerPool std::string owners. Full_Copy316 includes primary267 and catch49; no truncation, with EH10 and complete132-byte metadata. Actual Xran/Xlen string.obj and CxxThrowException throw.obj vendor identities. Distance old source10000.0f was rejected by full data check despite code candidate; corrected to actual100000.0f atEC54C. No incomplete provider or preview-renderer credit.',verification='Every whole code/data region, all actual target identities and relocation sets; eight old pending retained. Full gate, ten standard controls and actor ABI required. No new runtime execution or exception throw exercise.',reference_credit='337 direct025 adapted PlayerName bytes. Standard C++ library template implementations originate pinned SDK, not025; Entity and query methods direct025zero.')
new['review']['linker_alias_provider_evidence']=json.load(open('build/cp32-name-distance-linked2-aliases.json'))
Path('config/checkpoint32/name-distance-contract.json').write_text(json.dumps(new,indent=2)+'\n')
from verify_actor_closure import Gate
g=Gate('cp32-name-distance-linked2',contract_path='config/checkpoint32/name-distance-contract.json');g.bind()
new['review']['associative_comdat_bindings']={'run':'cp32-name-distance-linked2','bindings':g.associative_bindings,'negative_controls':json.load(open('build/associative-binding-controls.json')),'scope':'Previously tested strict associative-owner binding; all actual bytes and relocations still verified. Duplicate parent/EH gives no additional coverage.'}
Path('config/checkpoint32/name-distance-contract.json').write_text(json.dumps(new,indent=2)+'\n')
