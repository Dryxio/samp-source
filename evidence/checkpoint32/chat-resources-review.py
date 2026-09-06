import json,sys,struct
from pathlib import Path
sys.path.insert(0,'tools')
from binary import COFF,PE,sha
from verify_actor_closure import initial_bytes
old=json.load(open('config/checkpoint32/lifecycle-chat-contract.json'));new=json.load(open('build/actor-contract-draft.json'));prior={(r['rva'],r['size'],r['kind']):r for r in old['regions']}
run=Path('build/cp32-chat-resources-linked1');pe=PE('private/samp.dll');objs={p.stem:COFF(p) for p in run.glob('*.obj')}
reviews=[json.load(open('build/chat-resources-reviewed-manifest.json'))]
ids={f['symbol']:f['reference_va'] for r in old['regions'] for f in r['fixups']};ids.update({r['anchor']:pe.base+r['rva']+r['anchor_offset'] for r in old['regions']});ids.update({n:e['reference_va'] for n,e in old['externals'].items()})
expected={(r['unit'],r['rva'],r['size']):r for v in reviews for r in v['regions']}
for v in reviews:
 for u,names in v['seeds'].items():
  for n,rva in names.items():
   if n in ids:assert ids[n]==pe.base+rva,(n,ids[n],rva)
   ids[n]=pe.base+rva
for v in reviews:
 for n,va in v.get('external_targets',{}).items():
  if n in ids and not n.startswith('$'):assert ids[n]==va,(n,hex(ids[n]),hex(va))
seen=set()
for r in new['regions']:
 p=prior.get((r['rva'],r['size'],r['kind']))
 if p:
  assert r['sha256']==p['sha256'];assert [(f['offset'],f['kind'],f['reference_va']) for f in r['fixups']]==[(f['offset'],f['kind'],f['reference_va']) for f in p['fixups']];r['implementation']=p['implementation'];continue
 key=(r['unit'],r['rva'],r['size']);assert key in expected,('Unreviewed new region',r);seen.add(key);e=expected[key]
 o=objs[r['unit']];sec=o.sections[r['section']-1];assert sec['size']==r['size'] and r['offset']==0
 assert r['sha256']==e['sha256'];assert sha(initial_bytes(pe,r['rva'],r['size']))==e['sha256']
 wanted=[(f['site_rva']-r['rva'],f['kind'],f['symbol'],f['target_va']) for f in e['bindings']]
 actual=[(f['offset'],f['kind'],f['symbol'],(f['reference_va']+struct.unpack_from('<I',sec['bytes'],f['offset'])[0])&0xffffffff) for f in r['fixups']]
 assert actual==wanted,(key,actual,wanted)
 r['implementation']='Complete normal vendor C++ with actual InternalPacket, SplitPacketChannel and full ReliabilityLayer declarations' if r['kind']=='code' else 'Whole original source table collection/state section, no slicing or invented contiguous placement'
for key in expected:
 if key not in seen:assert any((r['rva'],r['size'])==key[1:] for r in old['regions']),('Missing reviewed section',key)
for n,e in new['externals'].items():
 if n not in old['externals']:
  imports={i['symbol']:i for i in reviews[0]['imports']};assert n in imports;i=imports[n]
  assert e['kind']=='import-thunk' and e['reference_va']==i['reference_va'] and e['import_slot_va']==i['iat_va'] and e['size']==6 and e['import_identity']==[i['module'],i['name']]
  assert e['sha256']==sha(pe.read(e['reference_va']-pe.base,6))
  from verify_actor_closure import import_slots,map_symbols
  assert import_slots(pe)[i['iat_va']]==(i['module'],i['name'])
  maps=map_symbols(run/'closure.map');assert any('d3dx9:d3dx9_25.dll' in provider for _,provider in maps[n]);e['library']=i['library']
  continue
 p=old['externals'][n];assert {k:v for k,v in p.items() if k not in ('import','library','chunks')}=={k:v for k,v in e.items() if k not in ('import','library','chunks')}
 for k in ('import','library','chunks'):
  if k in p:e[k]=p[k]
for u,a in old['weak_aliases'].items():assert new['weak_aliases'][u]==a
assert all(not new['weak_aliases'][u] for v in reviews for u in v['seeds'])
assert len(new['pending'])==5
for r in new['pending']:
 p=next(p for p in old['pending'] if p['anchor']==r['anchor']);r['reference_function_code_size']=p['reference_function_code_size']
 if 'limitation' in p:r['limitation']=p['limitation']
new['status']='REVIEWED';new['sdk']=json.load(open(run/'link.json',encoding='utf-8-sig'))['sdk'];assert all(new['sdk'][n]==h for n,h in old['sdk'].items())
new['review']=dict(old['review']);new['review'].update(scope='Chat resource creation419 and font measurement117:536 unique bytes.',adaptations='Normal R5 reconstruction, both functions exact first probe. Real CChatWindow::CreateFonts owner and nonallocating resources view with actual field offsets, dimensions and COM types. Preserve distinct texture-failure and render-to-surface-failure paths, actual configuration and debug providers.',verification='Eight preflight whole sections (two code/six strings),17 COFF/PE bindings; existing Y string reused without extra credit. True D3DXCreateTexture/CreateRenderToSurface import thunks, identities and pinned d3dx9.lib provider; zero source credit for imports. No header change, unchanged objects reused. Ten controls and actor ABI required.',reference_credit='Direct0250; cumulative21126.',initialization_limitation='Actual resource methods only, no native renderer executed. Chat constructor279 and Draw662 remain unmatched, dependent wrappers excluded; whole DLL incomplete.',excluded_candidates='No unused header constants or unrelated data sections; no dependency credit for pending constructor/draw.')
new['review']['linker_alias_provider_evidence']=json.load(open('build/cp32-chat-resources-linked1-aliases.json'))
p=Path('config/checkpoint32/chat-resources-contract.json');p.write_text(json.dumps(new,indent=2)+'\n')
from verify_actor_closure import Gate
g=Gate(run.name,contract_path=str(p));g.bind();new['review']['associative_comdat_bindings']={'run':run.name,'bindings':g.associative_bindings,'negative_controls':json.load(open('build/associative-binding-controls.json')),'scope':'Existing strict associative binding with all real bytes/targets checked and no duplicate credit.'};p.write_text(json.dumps(new,indent=2)+'\n')
print('Reviewed',len(seen),'new complete regions')
