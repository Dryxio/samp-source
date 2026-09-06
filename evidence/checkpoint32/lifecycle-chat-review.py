import json,sys,struct
from pathlib import Path
sys.path.insert(0,'tools')
from binary import COFF,PE,sha
from verify_actor_closure import initial_bytes
old=json.load(open('config/checkpoint32/packet-family-contract.json'));new=json.load(open('build/actor-contract-draft.json'));prior={(r['rva'],r['size'],r['kind']):r for r in old['regions']}
run=Path('build/cp32-lifecycle-chat-linked2');pe=PE('private/samp.dll');objs={p.stem:COFF(p) for p in run.glob('*.obj')}
reviews=[json.load(open('build/agent-independent/reliability-lifecycle-reviewed-manifest.json')),json.load(open('build/agent-textdraw/chat-render-text-reviewed-manifest.json')),json.load(open('build/chat-menu-reviewed-manifest.json'))]
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
  expected_crt={'??_L@YGXPAXIHP6EX0@Z1@Z':(0xc80cf,98,'LIBCMT:ehvecctr.obj'),'??_M@YGXPAXIHP6EX0@Z@Z':(0xc806f,96,'LIBCMT:ehvecdtr.obj')}
  assert n in expected_crt
  rva,size,owner=expected_crt[n];assert e['kind']=='crt' and e['reference_va']==pe.base+rva and e['size']==size and e['sha256']==sha(pe.read(rva,size))
  from verify_actor_closure import map_symbols
  maps=map_symbols(run/'closure.map');assert any(owner in provider for _,provider in maps[n])
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
new['review']=dict(old['review']);new['review'].update(scope='Complete ReliabilityLayer and InternalPacketPool lifecycle/reset plus chat RenderText851 and independent menu query14.',adaptations='Normal vendor C++ real ReliabilityLayer1794 and InternalPacketPool16/Packet55. Move sole SetPing91 and UpdateNextActionTime55 providers into lifecycle TU: restores InitializeVariables405 original scheduling;146 moved bytes give zero new credit. Whole selectany double512000.0 has visible constant value and separate8-byte COMDAT, avoiding mismatching pooled16-byte section. A separate extern-only constant lost original codegen and was rejected. Chat851 preserves actual stripped512 buffer and12 shadow passes; correct prototype added after initial compile error. Real nonallocating chat view. Menu14 original025 implementation.',verification='Whole functions, complete EH sections/unwind maps/constants, all COFF/PE targets and actual providers. Two real CRT vector lifetime helpers are external boundaries with zero source credit. Ten controls and actor ABI required. No headers changed; recompile changed units only.',reference_credit='Direct025 incremental14 for independently matched menu; chat851 hybrid and vendor lifecycle0. Cumulative21126.',initialization_limitation='Actual complete network class and pool lifetime source, no sockets or live packets run. ChatDraw662 and dependent wrappers remain excluded; no replacement DLL claim.',excluded_candidates='Unused pooled header constants and PING constant excluded. No empty FreeThreadedMemory homologue credit. Five old pending remain.')
new['review']['linker_alias_provider_evidence']=json.load(open('build/cp32-lifecycle-chat-linked2-aliases.json'))
p=Path('config/checkpoint32/lifecycle-chat-contract.json');p.write_text(json.dumps(new,indent=2)+'\n')
from verify_actor_closure import Gate
g=Gate(run.name,contract_path=str(p));g.bind();new['review']['associative_comdat_bindings']={'run':run.name,'bindings':g.associative_bindings,'negative_controls':json.load(open('build/associative-binding-controls.json')),'scope':'Existing strict associative binding with all real bytes/targets checked and no duplicate credit.'};p.write_text(json.dumps(new,indent=2)+'\n')
print('Reviewed',len(seen),'new complete regions')
