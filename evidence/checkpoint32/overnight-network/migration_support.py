"""Explicit owner migration controls for an isolated integration, no byte gate changes."""
import copy
from integration_pipeline import read, pinned, require

def migrate(base, items, expected):
    out=copy.deepcopy(base)
    keys=set()
    for m in items:
        key=(m['old_unit'],m['rva'],m['size'])
        require(key not in keys,'Duplicate migration');keys.add(key)
        rows=[r for r in out['regions'] if (r['unit'],r['rva'],r['size'])==key]
        require(len(rows)==1,'Missing original migration region')
        r=rows[0]
        require(r['anchor']==m['old_anchor'] and r['section']==m['old_section'],'Original owner mismatch')
        e=expected[(m['new_unit'],m['rva'],m['size'])]
        require(e['sha256']==r['sha256'],'Migration changes original bytes')
        require(e['anchor']==m['new_anchor'] and e['section']==m['new_section'],'New owner mismatch')
        require([(f['offset'],f['kind']) for f in r['fixups']]==[(f['site_rva']-r['rva'],f['kind']) for f in e['bindings']],'Migration relocation sites differ')
        r['unit']=m['new_unit']
    require(len({(r['unit'],r['rva'],r['size']) for r in out['regions']})==len(out['regions']),'Migration merges previously distinct regions')
    return out
