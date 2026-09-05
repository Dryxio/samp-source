"""Review historical builds against their own archived source snapshots.

This does not accept the current tree; tools/run.py is the acceptance gate.
"""
import json
from binary import COFF, PE, need, sha
from verify import ROOT, relocate

manifest=json.loads((ROOT/'config/manifest.json').read_text())
pe=PE(ROOT/'private/samp.dll',manifest['target_sha256'])
attempts=[]
for run in ('a','b','c'):
    directory=ROOT/'build'/run
    build=json.loads((directory/'build.json').read_text(encoding='utf-8-sig'))
    objects={}
    for unit in build['units']:
        name=unit['unit']
        need(sha((directory/'src'/f'{name}.cpp').read_bytes())==unit['source_sha256'],
             'historical source snapshot mismatch')
        path=directory/f'{name}.obj'
        need(sha(path.read_bytes())==unit['object_sha256'],'historical object mismatch')
        objects[name]=COFF(path)
    rows=[]
    for fn in manifest['functions']:
        try:
            code,_=relocate(objects[fn['unit']],fn,manifest,pe,pe.base)
            original=pe.read(fn['rva'],fn['size'])
            diffs=[i for i,(a,b) in enumerate(zip(code,original)) if a!=b]
            rows.append(dict(name=fn['name'],exact=not diffs,difference_offsets=diffs))
        except ValueError as error:
            rows.append(dict(name=fn['name'],exact=False,error=str(error)))
    count=sum(r['exact'] for r in rows)
    attempts.append(dict(run=run,matched=count,total=len(rows),functions=rows,build=build))
    print(f'Historical {run}: {count}/{len(rows)}')
(ROOT/'evidence/iterations.json').write_text(json.dumps(attempts,indent=2)+'\n')
