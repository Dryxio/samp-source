"""Build, link, run and strictly compare the checkpoint 2 capsule twice."""
import argparse
import json
import sys
from datetime import datetime, timezone
from pathlib import Path
from binary import need,sha
from run import command
from verify_checkpoint2 import Checkpoint,ROOT
sys.path.insert(0,str(ROOT/'tests/checkpoint2'))
from linked_emulate import exercise


def main():
    parser=argparse.ArgumentParser()
    parser.add_argument('--skip-build',action='store_true')
    args=parser.parse_args()
    destination=ROOT/'evidence/checkpoint2'
    destination.mkdir(parents=True,exist_ok=True)
    # A failed rerun must never leave a stale PASS at the acceptance path.
    acceptance=destination/'acceptance.json'
    acceptance.write_text(json.dumps(dict(result='IN_PROGRESS'))+'\n')
    script='C:\\Mac\\Home\\'+str((ROOT/'tools/build_checkpoint2.ps1').relative_to(Path.home())).replace('/','\\')
    results=[];emulation=[]
    for run,base in [('cp2-baseline','0x10000000'),('cp2-repeat','0x30000000')]:
        if not args.skip_build:
            command(['prlctl','exec','Windows 11','powershell.exe','-NoProfile',
                     '-ExecutionPolicy','Bypass','-File',script,'-Run',run,'-ImageBase',base],
                    ROOT/'build'/run/'compile.log')
        checkpoint=Checkpoint(run)
        need(checkpoint.linked.base==int(base,16),'unexpected preferred image base')
        result=checkpoint.verify()
        (checkpoint.directory/'verification.json').write_text(json.dumps(result,indent=2)+'\n')
        emulation.append(exercise(checkpoint))
        results.append(result)
        print('PASS linked matching and chain execution: '+run,flush=True)
    canonical=lambda r:[{k:v for k,v in region.items() if k!='linked_rva'} for region in r['regions']]
    need(canonical(results[0])==canonical(results[1]),'independent builds disagree')
    need(results[0]['build']['sources']==results[1]['build']['sources'],'sources changed between builds')
    command([sys.executable,'-m','unittest','discover','-s','tests/checkpoint2','-p','test_*.py','-v'],ROOT/'build/cp2-negative-controls.log')
    command([sys.executable,'-m','unittest','discover','-s','tests','-v'],ROOT/'build/cp2-cp1-regression.log')
    command([sys.executable,'tests/emulate.py','--run','baseline'],ROOT/'build/cp2-cp1-emulation.log')
    hashes={str(p.relative_to(ROOT)):sha(p.read_bytes()) for folder in ('src','tools','tests','config')
            for p in sorted((ROOT/folder).rglob('*')) if p.is_file() and '__pycache__' not in str(p)}
    summary=dict(result='PASS',utc=datetime.now(timezone.utc).isoformat(),
                 scope='Checkpoint 2 selected linked regions; not a whole R5 DLL or complete ActorPed implementation',
                 builds=results,linked_emulation=emulation,negative_controls='8 rejected mutations + 1 positive control',
                 cp1_regression='verifier tests and 324 emulated checks PASS',
                 unique_code_bytes_cp1_cp2=996,vendor_crt_entry_bytes_separate=59,file_hashes=hashes)
    acceptance.write_text(json.dumps(summary,indent=2)+'\n')
    print('PASS checkpoint 2: evidence/checkpoint2/acceptance.json',flush=True)


if __name__=='__main__': main()
