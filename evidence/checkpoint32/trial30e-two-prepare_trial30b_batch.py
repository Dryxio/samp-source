import json,sys
from pathlib import Path
sys.path.insert(0,'tools');from binary import COFF
s=json.load(open(sys.argv[1]));seeds=json.load(open('config/checkpoint32/'+s['previous']+'-seeds.json'));adds={}
for f in s['seed_files']:
 p=json.load(open(f))
 if isinstance(p,list):
  for r in p:adds.setdefault(r['unit'],{})[r['symbol']]=r['rva']
 else:
  for u,rows in p.items():adds.setdefault(u,{}).update(rows)
f=Path('config/checkpoint32/closure-exports.def');out=f.read_text();have={x.strip().split()[0] for x in out.splitlines() if x.strip()};new=[]
for u,rows in adds.items():
 o=COFF('build/'+s['ob1']+'/'+u+'.obj')
 for n,rva in rows.items():
  old=seeds.setdefault(u,{}).get(n);assert old is None or old==rva;seeds[u][n]=rva
  sym=next((v for v in o.names.get(n,[]) if v['section']>0),None);assert sym,(u,n)
  if sym['storage']==2 and n not in have:new.append('    '+n+'\n');have.add(n)
f.write_text(out+''.join(new));Path('config/checkpoint32/'+s['prefix']+'-seeds.json').write_text(json.dumps(seeds,indent=2)+'\n');print('Added exports',len(new))
