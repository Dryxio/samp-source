"""Propose whole raw matches; never modifies the acceptance contract."""
import argparse
from collections import defaultdict,Counter
import json
from pathlib import Path
from binary import COFF,PE,sha
from verify import ROOT
from verify_checkpoint31 import read_json


def discover(run):
    inventory=read_json(ROOT/'config/checkpoint31/inventory.json')
    named={f['name']:f for f in inventory['functions']};byhash=defaultdict(list)
    for f in inventory['functions']:
        if len(f['chunks'])==1:byhash[(f['chunks'][0]['size'],f['chunks'][0]['sha256'])].append(f)
    reference=PE(ROOT/'private/samp.dll',inventory['reference_sha256'])
    hits={};stats=Counter()
    for p in sorted((ROOT/'build'/run).glob('*.obj')):
        if not p.name.startswith(('raknet_','dxut_','utilities_')):continue
        obj=COFF(p)
        for name in obj.names:
            try:raw,relocs=obj.function(name)
            except ValueError:continue
            stats['complete_comdats']+=1
            if relocs:stats['require_relocation_contract']+=1;continue
            f=named.get(name);reason='IDA name plus complete bytes'
            if not f or len(f['chunks'])!=1 or f['chunks'][0]['size']!=len(raw) or sha(raw)!=f['chunks'][0]['sha256']:
                candidates=byhash[(len(raw),sha(raw))]
                if len(raw)<9 or len(candidates)!=1:stats['unmapped_or_ambiguous']+=1;continue
                f=candidates[0];reason='unique full body including exact IDA boundaries'
            if any(f['rva']<=site<f['rva']+len(raw) for site in reference.relocations):continue
            definitions=[s for s in obj.names[name] if s['section']>0 and s['type']==32]
            if definitions[0]['storage']!=2:continue
            hits.setdefault(f['rva'],dict(object=p.name,symbol=name,rva=f['rva'],size=len(raw),
                                         sha256=sha(raw),identity=reason,ida_name=f['name']))
    return dict(status='PROPOSALS_ONLY_NOT_ACCEPTED',statistics=dict(stats),regions=sorted(hits.values(),key=lambda h:h['rva']))


if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--run',default='cp31-baseline');a=p.parse_args()
    result=discover(a.run)
    output=ROOT/'build'/a.run/'proposals.json'
    output.write_text(json.dumps(result,indent=2)+'\n')
    print(len(result['regions']),'proposals;',output)
