import json,sys,struct
from pathlib import Path
sys.path.insert(0,'tools')
from binary import COFF,PE,sha
from verify_actor_closure import initial_bytes,Gate
old=json.load(open('config/checkpoint32/plate-request-contract.json'));new=json.load(open('build/actor-contract-draft.json'));prior={(r['rva'],r['size'],r['kind']):r for r in old['regions']}
run=Path('build/cp32-graph-label-socket-linked2');pe=PE('private/samp.dll');objs={p.stem:COFF(p) for p in run.glob('*.obj')}
reviews=[json.load(open(p)) for p in ['build/agent-independent/graph-subset-v2-reviewed-manifest.json','build/spatial-label-reviewed-normalized.json','build/socket-leaves-reviewed-manifest.json']]
expected={(r['unit'],r['rva'],r['size']):r for v in reviews for r in v['regions']};seen=set()
for r in new['regions']:
 p=prior.get((r['rva'],r['size'],r['kind']))
 if p:
  assert r['sha256']==p['sha256'];assert [(f['offset'],f['kind'],f['reference_va']) for f in r['fixups']]==[(f['offset'],f['kind'],f['reference_va']) for f in p['fixups']];r['implementation']=p['implementation'];continue
 key=(r['unit'],r['rva'],r['size']);assert key in expected,('Unreviewed new region',r);seen.add(key);e=expected[key]
 o=objs[r['unit']];sec=o.sections[r['section']-1];assert sec['size']==r['size'] and r['offset']==0
 assert r['sha256']==e['sha256']==sha(initial_bytes(pe,r['rva'],r['size']))
 wanted=[(f['site_rva']-r['rva'],f['kind'],f['symbol'],f['target_va']) for f in e['bindings']]
 actual=[(f['offset'],f['kind'],f['symbol'],(f['reference_va']+struct.unpack_from('<I',sec['bytes'],f['offset'])[0])&0xffffffff) for f in r['fixups']]
 assert actual==wanted,(key,actual,wanted)
 r['implementation']='Normal C++ graph templates with complete real Node/Graph/Map/Tree/Heap objects, spatial label8-byte owner or unchanged socket leaf; all original data/EH/providers verified'
for key in expected:
 if key not in seen:assert any((r['rva'],r['size'])==key[1:] for r in old['regions']),('Missing reviewed section',key)
for n,e in new['externals'].items():
 if n not in old['externals']:
  imports=json.load(open('build/socket-leaves-reviewed-manifest.json'))['imports'];assert n in imports and e['kind']=='import' and e['reference_va']==imports[n]['reference_va'],(n,e)
  e['import']=imports[n]['import'];continue
 p=old['externals'][n];assert {k:v for k,v in p.items() if k not in ('import','library','chunks')}=={k:v for k,v in e.items() if k not in ('import','library','chunks')}
 for k in ('import','library','chunks'):
  if k in p:e[k]=p[k]
for u,a in old['weak_aliases'].items():assert new['weak_aliases'][u]==a
assert all(not new['weak_aliases'][u] for v in reviews for u in v['seeds'])
assert len(new['pending'])==5
for r in new['pending']:
 p=next(p for p in old['pending'] if p['anchor']==r['anchor']);r['reference_function_code_size']=p['reference_function_code_size']
 if 'limitation' in p:r['limitation']=p['limitation']
new['status']='REVIEWED';new['sdk']=json.load(open(run/'link.json',encoding='utf-8-sig'))['sdk'];assert new['sdk']==old['sdk']
new['review']=dict(old['review']);new['review'].update(scope='Closed graph subset8157 principal code plus476 EH, spatial labels564 and socket leaves427; only unique union accepted.',adaptations='Reproduce real Node definition/declaration compilation contexts with MSVC extern template suppression and explicit real provider instantiations. All22 old bad lower providers absent from algorithm TU, every selected physical COMDAT copy reviewed. Two high algorithms now differ and SpanningTree depends on them; allthree excluded. Labels actual complete8-byte owner, true camera owner reused, full missing025 LOS descriptor18. Socket normal vendor leaves, real IAT providers; SendTo118 differs from R5 137 with added packet transformation so excluded.',reference_credit='Direct025 code0 cumulative21126; exact missing025 visibility descriptor18 supplies useful real data/semantics.',verification='Complete functions, EH maps804 and all COFF/PE target identities. Existing vector lifetime CRT providers unchanged; eleven actual wsock32 ordinal imports individually identified with zero source credit. Ten controls and actor ABI required; cached unchanged objects reused.',excluded_candidates='Dijkstra, ShortestPath, SpanningTree, SocketSendTo and all old five pending/parked excluded. No Router/transport/plugin closure or live execution claimed.')
new['review']['linker_alias_provider_evidence']=json.load(open('build/cp32-graph-label-socket-linked2-aliases.json'))
for name,file in [('server_bridge_final','server-bridge-target-alias-qualification-cp32-graph-label-socket-linked2.json'),('logger_deleting_aliases','logger-deleting-aliases-cp32-graph-label-socket-linked2.json')]:new['review'][name]=json.load(open('build/agent-independent/'+file))
p=Path('config/checkpoint32/graph-label-socket-contract.json');p.write_text(json.dumps(new,indent=2)+'\n')
g=Gate(run.name,contract_path=str(p));g.bind();new['review']['associative_comdat_bindings']={'run':run.name,'bindings':g.associative_bindings,'negative_controls':json.load(open('build/associative-binding-controls.json')),'scope':'Complete strict associative matching, no duplicate credit.'};p.write_text(json.dumps(new,indent=2)+'\n')
print('Reviewed',len(seen),'new complete regions')

new["review"]["comparator_resolution"]='Actual template comparator44 explicitly instantiated in both original codegen contexts. Removes compiler fallback to absent non-template name without changing gate. Both whole bodies and true Node::< / == targets verified, weak alias sets empty for all graph units. Four exact duplicate physical COMDAT seeds included with union-only credit.'
p.write_text(json.dumps(new,indent=2)+"\n")
