import json,sys,struct
from pathlib import Path
sys.path.insert(0,'tools')
from binary import COFF,PE,sha
from verify_actor_closure import initial_bytes,Gate
old=json.load(open('config/checkpoint32/graph-label-socket-contract.json'));new=json.load(open('build/actor-contract-draft.json'));prior={(r['rva'],r['size'],r['kind']):r for r in old['regions']}
run=Path('build/cp32-avl-string-label-linked1');pe=PE('private/samp.dll');objs={p.stem:COFF(p) for p in run.glob('*.obj')}
reviews=[json.load(open(p)) for p in ['build/agent-independent/network-id-tree-reviewed-manifest.json','build/agent-independent/string-table-reviewed-manifest.json','build/label-pool-foundations-reviewed-normalized.json']]
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
 r['implementation']='Normal complete C++ AVL16/node12/item12, real StringTable12/entry5 and whole label record29/pool10800, true providers and full state/EH'
for key in expected:
 if key not in seen:assert any((r['rva'],r['size'])==key[1:] for r in old['regions']),('Missing reviewed section',key)
for n,e in new['externals'].items():
 assert n in old['externals'],('Unexpected external',n,e)
 p=old['externals'][n];assert {k:v for k,v in p.items() if k not in ('import','library','chunks')}=={k:v for k,v in e.items() if k not in ('import','library','chunks')}
 for k in ('import','library','chunks'):
  if k in p:e[k]=p[k]
for u,a in old['weak_aliases'].items():
 if u=='closure_vendor_graph_playerid_comparisons':assert not a and u not in new['weak_aliases'];continue
 assert new['weak_aliases'][u]==a
for v in reviews:
 for u in v['seeds']:assert new['weak_aliases'][u]==v.get('actual_weak_aliases',{}).get(u,{})
assert len(new['pending'])==5
for r in new['pending']:
 p=next(p for p in old['pending'] if p['anchor']==r['anchor']);r['reference_function_code_size']=p['reference_function_code_size']
 if 'limitation' in p:r['limitation']=p['limitation']
new['status']='REVIEWED';new['sdk']=json.load(open(run/'link.json',encoding='utf-8-sig'))['sdk'];assert new['sdk']==old['sdk']
new['review']=dict(old['review']);new['review'].update(scope='AVL tree2359 including deleting60 and EH18; StringTable1474 including EH36; LabelPool New270 and remote activity29.',adaptations='Restore actual NetworkTypes.cpp method visibility by moving sole PlayerID comparison definitions into NetworkID comparison TU in original source order; preserves ECX knowledge and resolves86 vs84. Previous owner removed from build and seeds transferred; former bodies exact and no duplicate credit. Actual AVL/Tree16 and node/item12, real one-slot VFTs and static pointer4 locals plus modebool1. Whole StringTable12/entry5 lifetimes, true singleton/count8. Label record29 and pool10800 fully identified; no truncated allocation.',reference_credit='Direct0250 cumulative21126; all new code vendor or hybridR5, no fabricated source attribution.',verification='Whole functions, complete EH/maps/VFTs, every COFF/PE target and actual allocator/provider. Ten controls and actor ABI required. No header changes, cached unchanged sources reused.',excluded_candidates='NetworkIDGenerator virtual-authority APIs excluded. LabelPoolDraw643 and Transform92 still unmatched, pLabel scalar/main.cpp transfer remains pending and unselected. Existing five pending and other parked bodies excluded.')
new['review']['linker_alias_provider_evidence']=json.load(open('build/cp32-avl-string-label-linked1-aliases.json'))
for name,file in [('server_bridge_final','server-bridge-target-alias-qualification-cp32-avl-string-label-linked1.json'),('logger_deleting_aliases','logger-deleting-aliases-cp32-avl-string-label-linked1.json')]:new['review'][name]=json.load(open('build/agent-independent/'+file))
new['review']['network_tree_deleting_aliases']=json.load(open('build/agent-independent/network-id-tree-deleting-aliases-cp32-avl-string-label-linked1.json'))
p=Path('config/checkpoint32/avl-string-label-contract.json');p.write_text(json.dumps(new,indent=2)+'\n')
g=Gate(run.name,contract_path=str(p));g.bind();new['review']['associative_comdat_bindings']={'run':run.name,'bindings':g.associative_bindings,'negative_controls':json.load(open('build/associative-binding-controls.json')),'scope':'Complete strict associative matching, no duplicate credit.'};p.write_text(json.dumps(new,indent=2)+'\n')
print('Reviewed',len(seen),'new complete regions')
