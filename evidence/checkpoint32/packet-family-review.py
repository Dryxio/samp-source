import json,sys,struct
from pathlib import Path
sys.path.insert(0,'tools')
from binary import COFF,PE,sha
from verify_actor_closure import initial_bytes
old=json.load(open('config/checkpoint32/tea-health-contract.json'));new=json.load(open('build/actor-contract-draft.json'));prior={(r['rva'],r['size'],r['kind']):r for r in old['regions']}
run=Path('build/cp32-packet-family-linked1');pe=PE('private/samp.dll');objs={p.stem:COFF(p) for p in run.glob('*.obj')}
reviews=[json.load(open('build/agent-independent/reliability-packet-family-reviewed-manifest.json'))]
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
  assert n=='__alloca_probe' and e['kind']=='crt' and e['reference_va']==pe.base+0xc7eb0 and e['size']==61
  assert e['sha256']==sha(pe.read(0xc7eb0,61))
  from verify_actor_closure import symbol_address,map_symbols
  maps=map_symbols(run/'closure.map');va=symbol_address(maps,n)
  assert any('LIBCMT:chkstk.obj' in provider for _,provider in maps[n])
  linked=PE(run/'closure.dll');assert linked.read(va-linked.base,61)==pe.read(0xc7eb0,61)
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
new['review']=dict(old['review']);new['review'].update(scope='ReliabilityLayer complete split/reassembly, send/receive queues, ordering, statistics, ping and TEA setup:5919 unique source bytes including EH18.',adaptations='Extract original vendor methods with actual full class declarations. Initial extraction failed because a commented brace ended SplitPacket early; restored remaining original source. Corrected C++ emitted74/74 candidate functions, selected actual55-function dependency graph with whole EH18/xdata36. Real packet55, split channel20 and linked list12 layouts; actual allocations, releases and ordering structures.',verification='57 complete preflight sections,190 full COFF/PE bindings. Existing owner COMDATs preserved,558 previously covered bytes excluded. Real LIBCMT chkstk.obj alloca61 whole external verified, zero source credit. Ten controls and actor ABI required. No headers changed, all348 Ob1 objects reused from valid cache.',reference_credit='Baseline vendor source; direct0250 for batch, cumulative21112.',initialization_limitation='No whole ReliabilityLayer connection lifecycle claim; no live sockets, packets or GTA executed. Whole DLL incomplete.',excluded_candidates='Only selected root dependency closure; unrelated emitted helpers excluded. Old five pending stay excluded.')
new['review']['linker_alias_provider_evidence']=json.load(open('build/cp32-packet-family-linked1-aliases.json'))
p=Path('config/checkpoint32/packet-family-contract.json');p.write_text(json.dumps(new,indent=2)+'\n')
from verify_actor_closure import Gate
g=Gate(run.name,contract_path=str(p));g.bind();new['review']['associative_comdat_bindings']={'run':run.name,'bindings':g.associative_bindings,'negative_controls':json.load(open('build/associative-binding-controls.json')),'scope':'Existing strict associative binding with all real bytes/targets checked and no duplicate credit.'};p.write_text(json.dumps(new,indent=2)+'\n')
print('Reviewed',len(seen),'new complete regions')
