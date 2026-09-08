import json,sys,shutil
from pathlib import Path
from datetime import datetime,timezone
spec=json.load(open(sys.argv[1]));prefix=spec['prefix'];proof=json.load(open('evidence/checkpoint32/'+prefix+'-acceptance.json'));assert proof['result']=='PASS';c=proof['coverage'];assert c['previous_code_bytes']==spec['baseline'] and c['new_unique_code_bytes']==spec['expected_gain']
files=set(spec['manifests']+spec['seed_files']+list(spec['linked_reviews'].values())+spec.get('evidence_files',[])+[sys.argv[1],'build/prepare_trial30b_batch.py','build/trial30e/review_batch.py','build/qualify_trial30b_link.py','build/trial30e/finalize_batch.py'])
for file in files:
 p=Path(file);assert p.is_file(),p;shutil.copyfile(p,Path('evidence/checkpoint32')/(prefix+'-'+p.name))
summary=dict(status='ACCEPTED',run=spec['run'],ob1=spec['ob1'],ob2=spec['ob2'],coverage=c,review=spec['review'],checkpoint_complete=False,pushed=False,deployed=False)
Path('evidence/checkpoint32/'+prefix+'-integration.json').write_text(json.dumps(summary,indent=2)+'\n')
p=Path('README.md');s=p.read_text();old=f"{spec['baseline']:,} code bytes verified";assert old in s;import re
s=re.sub(re.escape(old)+r' — [0-9.]+%',f"{c['union_code_bytes']:,} code bytes verified — {c['text_percent']:.2f}%",s).replace(spec['previous']+'-acceptance.json',prefix+'-acceptance.json');p.write_text(s)
p=Path('evidence/checkpoint32/PROGRESS.md');p.write_text(p.read_text()+f"\n\n{prefix}: +{c['new_unique_code_bytes']} unique accepted R5 code bytes, total {c['union_code_bytes']}/930756 ({c['text_percent']:.6f}%). {spec['review']['scope']} Complete source/linked code,data,EH, actual COFF/PE targets, no unimplemented provider credit; required negative controls and Actor ABI PASS. Trial E continues until 2026-09-08T14:43:37Z; no push/deployment.\n")
p=Path('build/trial30e/state.json');s=json.loads(p.read_text());s.setdefault('batches',[]).append(dict(prefix=prefix,gain=c['new_unique_code_bytes'],coverage=c['union_code_bytes'],utc=datetime.now(timezone.utc).isoformat(),prepared_before_trial=spec.get('prepared_before_trial',0)));s['current_accepted']=c['union_code_bytes'];p.write_text(json.dumps(s,indent=2)+'\n');print(c)
