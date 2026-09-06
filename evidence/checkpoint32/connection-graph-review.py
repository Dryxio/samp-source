import json,sys,struct
from pathlib import Path
sys.path.insert(0,'tools')
from binary import COFF,PE,sha
from verify_actor_closure import initial_bytes,Gate
old=json.load(open('config/checkpoint32/nat-chat-mesh-contract.json'));new=json.load(open('build/actor-contract-draft.json'));prior={(r['rva'],r['size'],r['kind']):r for r in old['regions']}
run=Path('build/cp32-connection-graph-linked1');pe=PE('private/samp.dll');objs={p.stem:COFF(p) for p in run.glob('*.obj')}
reviews=[json.load(open('build/agent-independent/connection-graph-reviewed-manifest.json'))]
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
 r['implementation']='Normal C++ complete ConnectionGraph with actual node/template ownership and contiguous Packet allocation; whole attached data and true providers'
for key in expected:
 if key not in seen:assert any((r['rva'],r['size'])==key[1:] for r in old['regions']),('Missing reviewed section',key)
for n,e in new['externals'].items():
 if n not in old['externals']:
  raise AssertionError(('Unexpected new external',n,e))
 p=old['externals'][n];assert {k:v for k,v in p.items() if k not in ('import','library','chunks')}=={k:v for k,v in e.items() if k not in ('import','library','chunks')}
 for k in ('import','library','chunks'):
  if k in p:e[k]=p[k]
for u,a in old['weak_aliases'].items():
 if u=='closure_vendor_graph_node_identity':assert not a and u not in new['weak_aliases'];continue
 assert new['weak_aliases'][u]==a
for u,a in new['weak_aliases'].items():
 if u not in old['weak_aliases']:assert not a
assert len(new['pending'])==5
for r in new['pending']:
 p=next(p for p in old['pending'] if p['anchor']==r['anchor']);r['reference_function_code_size']=p['reference_function_code_size']
 if 'limitation' in p:r['limitation']=p['limitation']
new['status']='REVIEWED';new['sdk']=json.load(open(run/'link.json',encoding='utf-8-sig'))['sdk'];assert new['sdk']==old['sdk']
new['review']=dict(old['review']);new['review'].update(scope='Complete ConnectionGraph with AllocPacket32,10510 unique code bytes.',adaptations='Full vendor source and original template instantiations replace only node_identity owner. All101 earlier regions requalified, graph_algorithms57 unchanged with four actual duplicate copies exact. Real Graph90 and Packet21 plus payload allocation. Twenty-five unused nonmatching templates outside selected closure remain excluded; none duplicates accepted provider.',reference_credit='Direct025 new0, cumulative21988; existing vendor source.',verification='Only two new units and one owner consolidation, cached unchanged headers. Whole functions/data/EH and all COFF/PE targets, true providers, ten controls and Actor ABI required.',excluded_candidates='All old pending and parked functions unchanged. Chat navigation remains excluded because IsActive nonexact. High graph Dijkstra/Shortest/Spanning remain excluded.')
new['review']['crt_initializer_controls']=json.load(open('build/agent-independent/crt-initializer-binding-controls.json'))
new['review']['crt_initializer_independent_audit']=json.load(open('build/agent-independent/crt-initializer-pointer-independent-audit.json'))
new['review']['itoa_qualification']=json.load(open('build/connection-graph-itoa-qualified.json'))
new['review']['linker_alias_provider_evidence']=json.load(open('build/cp32-connection-graph-linked1-aliases.json'))
for name,file in [('server_bridge_final','server-bridge-target-alias-qualification-cp32-connection-graph-linked1.json'),('logger_deleting_aliases','logger-deleting-aliases-cp32-connection-graph-linked1.json')]:new['review'][name]=json.load(open('build/agent-independent/'+file))
new['review']['network_tree_deleting_aliases']=json.load(open('build/agent-independent/network-id-tree-deleting-aliases-cp32-connection-graph-linked1.json'))
new['review']['file_crt_qualification']=json.load(open('build/agent-independent/file-crt-seven-qualified.json'))
new['review']['sha_file_crt_qualification']=json.load(open('build/agent-independent/sha-file-crt-qualified.json'))
new['review']['sha1_deleting_alias_qualification']=json.load(open('build/connection-graph-sha1-deleting-alias-qualified.json'))
new['review']['prior_independent_batch_audit']=new['review'].pop('independent_batch_audit',None)
new['review']['historical_crt_scope']='CRT wrapper qualification snapshots retain their named accepted runs and exact SDK pins; current Gate rebinds all imported/CRT symbols to this link MAP.'
new['review']['mesh_deleting_alias_qualification']=json.load(open('build/agent-independent/mesh-deleting-aliases-cp32-connection-graph-linked1.json'))
new['review']['mesh_independent_audit']=json.load(open('build/agent-textdraw/mesh-independent-review.json'))
new['review']['latest_proof_coverage_sufficiency']=json.load(open('build/latest-coverage-sufficiency.json'))
new['review']['connection_graph_owner_transfer']=json.load(open('build/agent-independent/connection-graph-owner-transfer-reviewed.json'))
new['review']['connection_graph_scope']=json.load(open('build/agent-independent/connection-graph-scope-reviewed.json'))
p=Path('config/checkpoint32/connection-graph-contract.json');p.write_text(json.dumps(new,indent=2)+'\n')
g=Gate(run.name,contract_path=str(p));g.bind();new['review']['associative_comdat_bindings']={'run':run.name,'bindings':g.associative_bindings,'negative_controls':json.load(open('build/associative-binding-controls.json')),'scope':'Complete strict associative matching, no duplicate credit.'};new['review']['crt_initializer_bindings']=g.crt_initializer_bindings;p.write_text(json.dumps(new,indent=2)+'\n')
print('Reviewed',len(seen),'new complete regions')
