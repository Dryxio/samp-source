"""Inventory actual COFF owners and accepted regions; diagnostic, not acceptance."""
import argparse,json,struct
from pathlib import Path
from binary import COFF,sha

def owners(folder, regions):
    result={}
    for p in sorted(Path(folder).glob('*.obj')):
        c=COFF(p)
        for s in c.symbols.values():
            if s['storage']!=2 or s['section']<=0:continue
            sec=c.sections[s['section']-1]
            rel=[]
            for i in range(sec['reloc_count']):
                off,idx,kind=struct.unpack_from('<IIH',c.data,sec['reloc_ptr']+i*10)
                rel.append((off,kind,c.symbols[idx]['name']))
            accepted=[{'rva':r['rva'],'size':r['size'],'kind':r['kind'],'anchor':r['anchor']} for r in regions.get((p.stem,s['section']),[]) if r['offset']<=s['value']<r['offset']+r['size']]
            result.setdefault(s['name'],[]).append(dict(unit=p.stem,section=s['section'],value=s['value'],size=sec['size'],comdat=bool(sec['flags']&0x1000),section_hash=sha(sec['bytes']),relocations=rel,accepted=accepted))
    return result

def main():
    ap=argparse.ArgumentParser(description=__doc__);ap.add_argument('--contract',required=True);ap.add_argument('--run',required=True);ap.add_argument('--output',required=True);ap.add_argument('--candidate-run');ap.add_argument('--proof');ap.add_argument('--seed-file',action='append',default=[]);args=ap.parse_args()
    contract=json.loads(Path(args.contract).read_text());regions={}
    for r in contract['regions']:regions.setdefault((r['unit'],r['section']),[]).append(r)
    baseline=owners(args.run,regions)
    out=dict(status='OWNER_INVENTORY_NOT_ACCEPTANCE',contract=args.contract,run=args.run,symbols=baseline,accepted_regions=contract['regions'],externals=contract['externals'])
    if args.candidate_run:
        proposed=owners(args.candidate_run,{})
        collisions=[]
        for name,rows in proposed.items():
            for r in rows:
                if any(old['unit']==r['unit'] and (old['section_hash'],old['relocations'],old['value'])==(r['section_hash'],r['relocations'],r['value']) for old in baseline.get(name,[])):continue
                for old in baseline.get(name,[]):
                    if old['unit']==r['unit']:continue
                    identical=(old['section_hash'],old['relocations'],old['value'])==(r['section_hash'],r['relocations'],r['value'])
                    collisions.append(dict(symbol=name,candidate=r['unit'],existing=old['unit'],existing_accepted=old['accepted'],classification='IDENTICAL_COMDAT_REQUIRES_TARGET_REVIEW' if identical and old['comdat'] and r['comdat'] else 'REVIEW_COLLISION_BEFORE_LINK'))
        out['candidate_symbols']=proposed;out['collisions']=collisions
        print('Cross-owner collisions:',len(collisions),'requiring review:',sum(x['classification']=='REVIEW_COLLISION_BEFORE_LINK' for x in collisions))
    if args.proof:
        from rank_uncovered_candidates import accepted_code
        covered=accepted_code(args.proof);out['accepted_unique_code_bytes']=len(covered)
        out['root_coverage_checks']=[]
        for file in args.seed_file:
            rows=json.loads(Path(file).read_text())
            if not isinstance(rows,list):raise ValueError('seed-file needs explicit root sizes')
            for r in rows:
                need={'rva','size','symbol'}
                if not need.issubset(r):raise ValueError('seed root missing size/symbol/rva')
                n=len(set(range(r['rva'],r['rva']+r['size']))-covered)
                out['root_coverage_checks'].append(dict(symbol=r['symbol'],rva=r['rva'],size=r['size'],new_range_bytes=n,action='ALREADY_COVERED_REUSE_PROVIDER' if not n else 'PROPOSED_REQUIRES_COMPLETE_ACCEPTANCE'))
    Path(args.output).write_text(json.dumps(out,indent=2)+'\n');print('Symbols',len(baseline),'output',args.output)
if __name__=='__main__':main()
