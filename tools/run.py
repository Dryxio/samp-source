"""One-command pilot: independent VM builds, strict match, controls, emulation."""
import argparse
import json
import subprocess
import sys
from datetime import datetime, timezone
from pathlib import Path

from binary import need, sha
from verify import ROOT, verify


def command(args, log):
    result = subprocess.run(args,cwd=ROOT,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,
                            text=True,errors='replace',timeout=120)
    log.parent.mkdir(parents=True,exist_ok=True)
    log.write_text(result.stdout)
    print(result.stdout,end='',flush=True)
    need(result.returncode == 0,'command failed; see '+str(log))


def main():
    parser=argparse.ArgumentParser()
    parser.add_argument('--skip-build',action='store_true',help='Revalidate already produced artifacts')
    args=parser.parse_args()
    results=[]
    script='C:\\Mac\\Home\\'+str((ROOT/'tools/build.ps1').relative_to(Path.home())).replace('/','\\')
    for run in ('baseline','repeat'):
        if not args.skip_build:
            command(['prlctl','exec','Windows 11','powershell.exe','-NoProfile',
                     '-ExecutionPolicy','Bypass','-File',script,'-Run',run],ROOT/'build'/run/'compile.log')
        result=verify(run)
        (ROOT/'build'/run/'verification.json').write_text(json.dumps(result,indent=2)+'\n')
        need(result['result']=='PASS',f'{run}: matching failed; see verification.json')
        print(f"PASS {run}: {result['functions_matched']} functions, {result['unique_code_bytes']} bytes",flush=True)
        results.append(result)
    need(results[0]['functions']==results[1]['functions'],'independent builds disagree')
    need(results[0]['build']['toolchain']==results[1]['build']['toolchain'],'toolchain changed between builds')
    for key in ('options','units'):
        if key=='units':
            a={u['unit']:u['source_sha256'] for u in results[0]['build'][key]}
            b={u['unit']:u['source_sha256'] for u in results[1]['build'][key]}
            need(a==b,'source changed between builds')
        else:
            need(results[0]['build'][key]==results[1]['build'][key],'options changed between builds')
    command([sys.executable,'-m','unittest','discover','-s','tests','-v'],ROOT/'build/negative-controls.log')
    command([sys.executable,'tests/emulate.py','--run','baseline'],ROOT/'build/emulation.log')
    emulation=json.loads((ROOT/'build/baseline/emulation.json').read_text())
    scripts={str(p.relative_to(ROOT)):sha(p.read_bytes()) for folder in ('src','tools','tests','config')
             for p in sorted((ROOT/folder).rglob('*')) if p.is_file() and '__pycache__' not in str(p)}
    summary=dict(result='PASS',utc=datetime.now(timezone.utc).isoformat(),
                 statement='Pilot validated; no complete DLL or in-game replacement claim',
                 functions=results[0]['functions'],functions_matched=results[0]['functions_matched'],
                 unique_code_bytes=results[0]['unique_code_bytes'],
                 reference_text_bytes=results[0]['reference_text_bytes'],
                 reference_sha256=results[0]['target_sha256'],
                 independent_builds=[r['build'] for r in results],
                 negative_controls='PASS; see tests/test_verifier.py and build/negative-controls.log',
                 emulation={'result':emulation['result'],'checks':emulation['checks'],
                            'environment':emulation['environment'],'mocks':emulation['mocks']},
                 external_dependencies=results[0]['external_dependencies'],file_hashes=scripts)
    (ROOT/'evidence/acceptance.json').write_text(json.dumps(summary,indent=2)+'\n')
    print('PASS pilot acceptance: evidence/acceptance.json',flush=True)


if __name__=='__main__':
    try:
        main()
    except (ValueError,OSError,subprocess.TimeoutExpired) as error:
        print('FAIL:',error,file=sys.stderr)
        raise SystemExit(1)
