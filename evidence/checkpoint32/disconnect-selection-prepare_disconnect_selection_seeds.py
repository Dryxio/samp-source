import json,sys
from pathlib import Path
sys.path.insert(0,'tools');from binary import COFF
seeds=json.load(open('config/checkpoint32/send-font-seeds.json'));adds={}
for file in ['build/agent-independent/rakpeer-disconnect-requests-seeds.json']:
 for u,rows in json.load(open(file)).items():adds.setdefault(u,{}).update(rows)
for file in ['build/agent-textdraw/selection-ped-seeds.json','build/agent-textdraw/vehicle-selection-seeds.json']:
 for r in json.load(open(file)):adds.setdefault(r['unit'],{})[r['symbol']]=r['rva']
exports=Path('config/checkpoint32/closure-exports.def');s=exports.read_text();have={x.strip().split()[0] for x in s.splitlines() if x.strip()};lines=[]
for u,rows in adds.items():
 path=Path('build/cp32-disconnect-selection-ob1')/(u+'.obj')
 if not path.exists():path=Path('build/cp32-disconnect-selection-ob2')/(u+'.obj')
 o=COFF(path)
 for n,rva in rows.items():
  old=seeds.setdefault(u,{}).get(n);assert old is None or old==rva;seeds[u][n]=rva
  sym=next((v for v in o.names.get(n,[]) if v['section']>0),None);assert sym,(u,n)
  if sym['storage']==2 and n not in have:lines.append('    '+n+'\n');have.add(n)
exports.write_text(s+''.join(lines));Path('config/checkpoint32/disconnect-selection-seeds.json').write_text(json.dumps(seeds,indent=2)+'\n');print('Added exports',len(lines))
