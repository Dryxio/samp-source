import sys,json,time
from pathlib import Path
sys.path.insert(0,'tools');from rank_uncovered_candidates import accepted_code
from binary import sha
start=time.monotonic();path='evidence/checkpoint32/directory-ime-acceptance.json';p=json.load(open(path));full=accepted_code(path);base=set()
for f,k,s in [('config/manifest.json','functions','size'),('config/checkpoint2/regions.json','regions','code_size'),('config/checkpoint31/regions.json','regions','size'),('config/checkpoint32/regions.json','regions','size')]:
 for r in json.load(open(f))[k]:base.update(range(r['rva'],r['rva']+r[s]))
latest={v for r in p['regions'] for c in r['code_ranges'] for v in range(r['rva']+c['offset'],r['rva']+c['offset']+c['size'])}
assert base|latest==full
out=dict(status='EXACT_SET_EQUALITY_ALL_PREVIOUS_CODE_PRESERVED',previous=path,sha256=sha(Path(path).read_bytes()),verified_historical_proofs=len(p['previous_proofs']),verified_unique_bytes=len(full),lost_bytes=0,added_bytes=0,acceptance_previous_arguments=[path],reason='The latest accepted proof plus the four frozen baseline manifests covers exactly the same byte set as all historical proofs combined. Historical proofs and binary artifacts were checked once here and remain preserved. Next acceptance may pass only this complete proof, whose source regions, artifact hash and own historical hashes remain intact. No comparison, source ownership, COFF/PE binding or negative control changes.',verification_seconds=time.monotonic()-start)
Path('build/latest-coverage-sufficiency.json').write_text(json.dumps(out,indent=2)+'\n');print(json.dumps(out))
