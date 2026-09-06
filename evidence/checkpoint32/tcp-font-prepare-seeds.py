import json,sys
from pathlib import Path
sys.path.insert(0,'tools');from binary import COFF,PE,sha
from verify_actor_closure import initial_bytes
seeds=json.load(open('config/checkpoint32/replica-navigation-seeds.json'));adds=json.load(open('build/agent-independent/tcp-seeds.json'))
for r in json.load(open('build/agent-textdraw/font-helpers-seeds.json')):adds.setdefault(r['unit'],{})[r['symbol']]=r['rva']
stores=[('embedded_color','?r5FontEmbeddedColor@@3KA',0x136e90,4),('character_index','?r5FontCharacterIndex@@3HA',0x136ea4,4),('wide_text','?r5FontWideText@@3PAGA',0x136ea8,40000),('original_text','?r5FontOriginalText@@3PADA',0x140aec,4)]
pe=PE('private/samp.dll');review=[]
for suffix,n,rva,size in stores:
 u='closure_store_font_'+suffix;o=COFF(Path('build/cp32-tcp-font-ob1')/(u+'.obj'));sym=next(x for x in o.names[n] if x['section']>0);sec=o.sections[sym['section']-1];assert sec['uninitialized'] and sec['size']==size and sym['value']==0 and sec['reloc_count']==0
 assert initial_bytes(pe,rva,size)==bytes(size);adds[u]={n:rva};review.append(dict(unit=u,anchor=n,rva=rva,size=size,kind='zero',sha256=sha(bytes(size)),bindings=[],object_sha256=sha(o.data),source_credit=0))
exports=Path('config/checkpoint32/closure-exports.def');s=exports.read_text();have={x.strip().split()[0] for x in s.splitlines() if x.strip()};lines=[]
for u,rows in adds.items():
 o=COFF(Path('build/cp32-tcp-font-ob1')/(u+'.obj'))
 for n,rva in rows.items():
  old=seeds.setdefault(u,{}).get(n);assert old is None or old==rva;seeds[u][n]=rva
  sym=next((v for v in o.names.get(n,[]) if v['section']>0),None);assert sym,(u,n)
  if sym['storage']==2 and n not in have:lines.append('    '+n+'\n');have.add(n)
exports.write_text(s+''.join(lines));Path('config/checkpoint32/tcp-font-seeds.json').write_text(json.dumps(seeds,indent=2)+'\n');Path('build/font-stores-reviewed.json').write_text(json.dumps(dict(status='WHOLE_TRUE_BSS_STORES_COMPILED',regions=review,source_credit=0,limitation='Only actual static pointer/scratch state represented; startup allocator C4790 and whole heap buffer lifecycle are unimplemented and not counted.'),indent=2)+'\n');print('Added exports',len(lines))
