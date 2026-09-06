import json,sys,struct
from pathlib import Path
sys.path.insert(0,'tools')
from binary import COFF,PE,sha
from verify_actor_closure import initial_bytes,Gate
old=json.load(open('config/checkpoint32/connection-graph-contract.json'));new=json.load(open('build/actor-contract-draft.json'));prior={(r['rva'],r['size'],r['kind']):r for r in old['regions']}
run=Path('build/cp32-replica-navigation-linked3');pe=PE('private/samp.dll');objs={p.stem:COFF(p) for p in run.glob('*.obj')}
reviews=[json.load(open(p)) for p in ['build/agent-independent/replica-network-id-reviewed-manifest.json','build/agent-independent/replica-network-id-inequality-reviewed.json','build/network-id-mode-reviewed.json','build/agent-textdraw/chat-navigation-reviewed-manifest.json']]
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
 r['implementation']='Normal C++ complete ReplicaManager/NetworkIDGenerator and chat navigation with true providers, initializers and full attached data'
for key in expected:
 if key not in seen:assert any((r['rva'],r['size'])==key[1:] for r in old['regions']),('Missing reviewed section',key)
for n,e in new['externals'].items():
 if n not in old['externals']:
  raise AssertionError(('Unexpected new external',n,e))
 p=old['externals'][n];assert {k:v for k,v in p.items() if k not in ('import','library','chunks')}=={k:v for k,v in e.items() if k not in ('import','library','chunks')}
 for k in ('import','library','chunks'):
  if k in p:e[k]=p[k]
for u,a in old['weak_aliases'].items():
 if u=='closure_vendor_network_id_tree':
  assert u not in new['weak_aliases']
  assert all(new['weak_aliases']['closure_vendor_network_id_generator'].get(k)==v for k,v in a.items());continue
 assert new['weak_aliases'][u]==a
for u,a in new['weak_aliases'].items():
 if u not in old['weak_aliases']:
  if u=='closure_vendor_network_id_generator':assert a==reviews[0]['actual_weak_aliases'][u]
  elif u=='closure_chat_ime_interaction':assert a=={'_r5ImeShowReadingWindowImport':'?s_bShowReadingWindow@CDXUTIMEEditBox@@1_NA'}
  else:assert not a
assert len(new['pending'])==5
for r in new['pending']:
 p=next(p for p in old['pending'] if p['anchor']==r['anchor']);r['reference_function_code_size']=p['reference_function_code_size']
 if 'limitation' in p:r['limitation']=p['limitation']
new['status']='REVIEWED';new['sdk']=json.load(open(run/'link.json',encoding='utf-8-sig'))['sdk'];assert new['sdk']==old['sdk']
new['review']=dict(old['review']);new['review'].update(scope='ReplicaManager/NetworkIDGenerator11791, actual NetworkID inequality75/getter6, chat navigation259.',adaptations='Full vendor sources replace only tree owner, all35 old regions retained. Real Generator17, Replica17, Manager48 and callbacks; complete BSS28, two initializer bodies47, destructor10 and compiler CRT registration block8. New whole-array binder preserves original and linked pointers/PE without splitting source section. Chat IsActive42 uses uniform bool control flow, genuine import-only alias to existing bool1 and exact page navigation.',reference_credit='Direct025 new0, cumulative21988; existing vendor and R5 UI reconstruction.',verification='Only four new units plus getter6, no header change. All full regions and COFF/PE targets, explicit true aliases, ten controls and Actor ABI required. CRT block8 and scalar4 pass two positive and eight targeted negative controls; existing4 also passes all five prior faults with current verifier.',excluded_candidates='Five old pending and parked IME handlers unchanged. No concrete Replica callbacks/authority implementations or CRT code credited.')
new['review']['crt_initializer_block_controls']=json.load(open('build/agent-textdraw/crt-initializer-block-controls.json'))
new['review']['crt_initializer_controls']=json.load(open('build/crt-initializer-four-controls-current.json'))
new['review']['crt_initializer_independent_audit']=json.load(open('build/agent-independent/crt-initializer-pointer-independent-audit.json'))
new['review']['itoa_qualification']=json.load(open('build/replica-navigation-itoa-qualified.json'))
new['review']['linker_alias_provider_evidence']=json.load(open('build/cp32-replica-navigation-linked3-aliases.json'))
for name,file in [('server_bridge_final','server-bridge-target-alias-qualification-cp32-replica-navigation-linked3.json'),('logger_deleting_aliases','logger-deleting-aliases-cp32-replica-navigation-linked3.json')]:new['review'][name]=json.load(open('build/agent-independent/'+file))
new['review']['network_tree_deleting_aliases']=json.load(open('build/agent-independent/replica-network-id-aliases-qualified.json'))
new['review']['file_crt_qualification']=json.load(open('build/agent-independent/file-crt-seven-qualified.json'))
new['review']['sha_file_crt_qualification']=json.load(open('build/agent-independent/sha-file-crt-qualified.json'))
new['review']['sha1_deleting_alias_qualification']=json.load(open('build/replica-navigation-sha1-deleting-alias-qualified.json'))
new['review']['prior_independent_batch_audit']=new['review'].pop('independent_batch_audit',None)
new['review']['historical_crt_scope']='CRT wrapper qualification snapshots retain their named accepted runs and exact SDK pins; current Gate rebinds all imported/CRT symbols to this link MAP.'
new['review']['mesh_deleting_alias_qualification']=json.load(open('build/agent-independent/mesh-deleting-aliases-cp32-replica-navigation-linked3.json'))
new['review']['mesh_independent_audit']=json.load(open('build/agent-textdraw/mesh-independent-review.json'))
new['review']['latest_proof_coverage_sufficiency']=json.load(open('build/latest-coverage-sufficiency.json'))
new['review']['replica_network_id_owner_transfer']=json.load(open('build/agent-independent/replica-network-id-owner-transfer-reviewed.json'))
new['review']['replica_network_id_scope']=json.load(open('build/agent-independent/replica-network-id-scope-reviewed.json'))
new['review']['chat_navigation_alias']=json.load(open('build/agent-textdraw/chat-navigation-linked-alias-review.json'))
p=Path('config/checkpoint32/replica-navigation-contract.json');p.write_text(json.dumps(new,indent=2)+'\n')
g=Gate(run.name,contract_path=str(p));g.bind();new['review']['associative_comdat_bindings']={'run':run.name,'bindings':g.associative_bindings,'negative_controls':json.load(open('build/associative-binding-controls.json')),'scope':'Complete strict associative matching, no duplicate credit.'};new['review']['crt_initializer_bindings']=g.crt_initializer_bindings;p.write_text(json.dumps(new,indent=2)+'\n')
print('Reviewed',len(seen),'new complete regions')
