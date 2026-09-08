import json,sys,struct
from pathlib import Path
sys.path.insert(0,'tools')
from binary import COFF,PE,sha
from verify_checkpoint2 import import_slots
from verify_actor_closure import initial_bytes,Gate
spec=json.load(open(sys.argv[1]));old=json.load(open('config/checkpoint32/'+spec['previous']+'-contract.json'));new=json.load(open(spec['draft_file']));prior={(r['unit'],r['rva'],r['size'],r['kind']):r for r in old['regions']}
run=Path('build')/spec['run'];pe=PE('private/samp.dll');objs={p.stem:COFF(p) for p in run.glob('*.obj')}
reviews=[json.load(open(p)) for p in spec['manifests']]
expected={(r['unit'],r['rva'],r['size']):r for v in reviews for r in v['regions']};seen=set();preserved=set()
for r in new['regions']:
 key=(r['unit'],r['rva'],r['size'],r['kind']);p=prior.get(key)
 if p:
  assert r['sha256']==p['sha256'];assert [(f['offset'],f['kind'],f['reference_va']) for f in r['fixups']]==[(f['offset'],f['kind'],f['reference_va']) for f in p['fixups']];r['implementation']=p['implementation'];preserved.add(key);continue
 key=key[:3];assert key in expected,('Unreviewed new region',r);seen.add(key);e=expected[key]
 o=objs[r['unit']];sec=o.sections[r['section']-1];assert sec['size']==r['size'] and r['offset']==0
 assert r['sha256']==e['sha256']==sha(initial_bytes(pe,r['rva'],r['size']))
 wanted=[(f['site_rva']-r['rva'],f['kind'],f['symbol'],f['target_va']) for f in e['bindings']]
 actual=[(f['offset'],f['kind'],f['symbol'],(f['reference_va']+struct.unpack_from('<I',sec['bytes'],f['offset'])[0])&0xffffffff) for f in r['fixups']]
 assert actual==wanted,(key,actual,wanted)
 r['implementation']=spec['review'].get('implementation','Normal C++; whole source regions and actual targets')
assert preserved==set(prior)
for key in expected:
 if key not in seen and expected[key]['kind']!='.CRT$XCU':assert any((r['rva'],r['size'])==key[1:] for r in old['regions']),('Missing reviewed section',key)
for n,e in new['externals'].items():
 if n in old['externals']:
  p=old['externals'][n];assert {k:v for k,v in p.items() if k not in ('import','library','chunks')}=={k:v for k,v in e.items() if k not in ('import','library','chunks')}
  for k in ('import','library','chunks'):
   if k in p:e[k]=p[k]
 else:
  raise AssertionError(('Unreviewed external',n,e))
assert set(new['externals'])==set(old['externals'])
for u,a in old['weak_aliases'].items():
 if new['weak_aliases'][u]!=a:
  review=spec.get('reviewed_alias_changes',{}).get(u);assert review and review['old']==a and review['new']==new['weak_aliases'][u],('Unreviewed changed alias',u)
for u,a in new['weak_aliases'].items():
 if u not in old['weak_aliases']:
  wanted={}
  for review in reviews:wanted.update(review.get('actual_weak_aliases',{}).get(u,{}))
  assert a==wanted,(u,a,wanted)
assert len(new['pending'])==len(old['pending'])
for r in new['pending']:
 p=next(p for p in old['pending'] if p['anchor']==r['anchor']);r['reference_function_code_size']=p['reference_function_code_size']
 if 'limitation' in p:r['limitation']=p['limitation']
new['status']='REVIEWED';new['sdk']=json.load(open(run/'link.json',encoding='utf-8-sig'))['sdk'];assert new['sdk']==old['sdk']
new['review']=dict(old['review']);new['review'].update(spec['review'])
for name,file in spec['linked_reviews'].items():new['review'][name]=json.load(open(file))
p=Path('config/checkpoint32')/(spec['prefix']+'-contract.json');p.write_text(json.dumps(new,indent=2)+'\n')
g=Gate(run.name,contract_path=str(p));g.bind();new['review']['associative_comdat_bindings']={'run':run.name,'bindings':g.associative_bindings,'negative_controls':json.load(open('build/associative-binding-controls.json')),'scope':'Complete strict associative matching, no duplicate credit.'};new['review']['crt_initializer_bindings']=g.crt_initializer_bindings;p.write_text(json.dumps(new,indent=2)+'\n');print('Reviewed',len(seen),'new whole sections, all',len(prior),'old preserved')

for key,e in expected.items():
 if e['kind']=='.CRT$XCU':
  rows=[b for b in g.crt_initializer_bindings if (b['unit'],b['original_rva'],b['size'])==key];assert len(rows)==1,(key,rows)
  b=rows[0];assert b['section']==e['section']
  assert [(x['offset'],x['provider']) for x in b['providers']]==[(x['site_rva']-e['rva'],x['symbol']) for x in e['bindings']]
print('Explicit reviewed CRT blocks matched through strict initializer binder')
