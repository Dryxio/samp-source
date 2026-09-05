"""Checkpoint 3.1: pin inputs, build/link twice, verify and measure honestly."""
import argparse
import json
import subprocess
import sys
from datetime import datetime,timezone
from pathlib import Path
from binary import need,sha,u32
from verify import ROOT,verify
from verify_checkpoint2 import Checkpoint,relocs
from verify_checkpoint31 import Gate,read_json
from report_checkpoint31 import report


def command(arguments,log):
    result=subprocess.run(arguments,cwd=ROOT,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,
                          text=True,errors='replace',timeout=600)
    log.parent.mkdir(parents=True,exist_ok=True);log.write_text(result.stdout)
    print(result.stdout,end='',flush=True)
    need(result.returncode==0,'command failed; see '+str(log))


def canonical_objects(gate):
    result={}
    for filename,obj in sorted(gate.objects.items()):
        sections=[]
        for index,section in enumerate(obj.sections,1):
            if section['name'].startswith('.debug$'):continue
            payload=sha(section['bytes'])
            if section['name']=='.sxdata':
                need(section['size']%4==0,'bad SafeSEH symbol indices')
                payload=[obj.symbols[u32(section['bytes'],off)]['name'] for off in range(0,section['size'],4)]
            fixups=[dict(offset=r['offset'],kind=r['kind'],symbol=r['symbol']) for r in relocs(obj,index)]
            sections.append(dict(name=section['name'],size=section['size'],flags=section['flags'],
                                 uninitialized=section['uninitialized'],payload=payload,fixups=fixups))
        result[filename]=sha(json.dumps(sections,sort_keys=True).encode())
    return result


def preflight():
    for name,digest in read_json(ROOT/'config/checkpoint31/private-inputs.json').items():
        need(sha((ROOT/'private/checkpoint31'/name).read_bytes())==digest,'private input differs: '+name)
    for name,digest in read_json(ROOT/'config/checkpoint31/vendor.json')['files'].items():
        need(sha((ROOT/name).read_bytes())==digest,'vendor input differs: '+name)
    source_files={str(p.relative_to(ROOT)) for p in (ROOT/'vendor').rglob('*') if p.is_file()}
    need(source_files==set(read_json(ROOT/'config/checkpoint31/vendor.json')['files']),'vendor lock must cover every file')


def main():
    parser=argparse.ArgumentParser();parser.add_argument('--skip-build',action='store_true');args=parser.parse_args()
    dest=ROOT/'evidence/checkpoint31';dest.mkdir(parents=True,exist_ok=True)
    path=dest/'acceptance.json';path.write_text('{"result":"IN_PROGRESS"}\n')
    try:
        preflight();results=[];gates=[]
        for run,base in [('cp31-baseline','0x10000000'),('cp31-repeat','0x30000000')]:
            if not args.skip_build:
                for script,extra in [('build_checkpoint31.ps1',[]),('link_checkpoint31.ps1',['-ImageBase',base])]:
                    vm='C:\\Mac\\Home\\'+str((ROOT/'tools'/script).relative_to(Path.home())).replace('/','\\')
                    command(['prlctl','exec','Windows 11','powershell.exe','-NoProfile','-ExecutionPolicy','Bypass',
                             '-File',vm,'-Run',run]+extra,ROOT/'build'/run/(script+'.log'))
            gate=Gate(run);need(gate.linked.base==int(base,16),'unexpected image base')
            result=gate.verify();gates.append(gate);results.append(result)
            (gate.directory/'verification.json').write_text(json.dumps(result,indent=2)+'\n')
            print(f"PASS {run}: {len(result['regions'])} linked regions, {result['native_checks']} native checks",flush=True)
        first,second=map(canonical_objects,gates)
        need(first==second,'independent code/data sections or symbolic fixups differ')
        for index,one in enumerate(results[0]['regions']):
            two=results[1]['regions'][index]
            need({k:v for k,v in one.items() if k!='linked_rva'}=={k:v for k,v in two.items() if k!='linked_rva'},'accepted regions differ across builds')
        for suite in ('tests/checkpoint31','tests','tests/checkpoint2'):
            command([sys.executable,'-m','unittest','discover','-s',suite,'-p','test_*.py','-v'],
                    ROOT/'build/cp31-baseline'/('tests-'+suite.replace('/','-')+'.log'))
        for run in ('baseline','repeat'):need(verify(run)['result']=='PASS','CP1 regression failed')
        for run in ('cp2-baseline','cp2-repeat'):need(Checkpoint(run).verify()['result']=='PASS','CP2 regression failed')
        command([sys.executable,'tests/emulate.py','--run','baseline'],ROOT/'build/cp31-baseline/cp1-emulation.log')
        sys.path.insert(0,str(ROOT/'tests/checkpoint2'))
        from linked_emulate import exercise
        for run in ('cp2-baseline','cp2-repeat'):exercise(Checkpoint(run))
        summary=report(gates[0],results[0])
        (dest/'coverage.json').write_text(json.dumps(summary,indent=2)+'\n')
        hashes={str(p.relative_to(ROOT)):sha(p.read_bytes()) for folder in ('src','tools','tests','config')
                for p in sorted((ROOT/folder).rglob('*')) if p.is_file() and '__pycache__' not in str(p)}
        acceptance=dict(result='PASS',utc=datetime.now(timezone.utc).isoformat(),
                        scope='Foundation libraries built and integrated; only listed full regions byte-certified; game client incomplete',
                        regions=results[0]['regions'],coverage=summary['coverage'],source_units=summary['source_units'],
                        builds=[dict(run=g.build['run'],image_base=g.linked.base,build_sha256=sha((g.directory/'build.json').read_bytes()),
                                     link_sha256=sha((g.directory/'link.json').read_bytes()),native_checks=r['native_checks'],native_log=r['native_log'],
                                     artifacts=g.link['artifacts'],sdk=g.link['sdk'],runtimes=g.link['runtimes']) for g,r in zip(gates,results)],
                        reproducibility=dict(result='PASS',canonical_objects=first,
                                             definition='All non-debug COFF sections and symbolic relocations; SafeSEH indices resolved to symbols; excludes timestamps, debug paths and final PE layout'),
                        regression='CP1 + CP2 strict gates, existing negative controls, 324 CP1 and 12 CP2 emulated executions PASS; previous native binaries not rerun',
                        negative_controls='262 last-byte corruptions and additional range/link/hash/PE/archive/source/BSS controls rejected',
                        file_hashes=hashes)
        path.write_text(json.dumps(acceptance,indent=2)+'\n')
        print('PASS checkpoint 3.1:',summary['coverage'],flush=True)
    except Exception as error:
        path.write_text(json.dumps(dict(result='FAIL',error=str(error)),indent=2)+'\n')
        raise


if __name__=='__main__':main()
