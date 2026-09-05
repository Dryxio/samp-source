"""Build the current CP3.2 batch twice; fail closed and publish measured coverage."""
import argparse,json,os,re,subprocess,sys
from pathlib import Path
from datetime import datetime,timezone
from binary import need,sha
from verify import ROOT,verify
from verify_checkpoint2 import Checkpoint
from verify_checkpoint31 import Gate as Foundation,read_json
from verify_checkpoint32 import Gate
from run_checkpoint31 import canonical_objects,preflight
from prepare_checkpoint32 import prepare


def command(args,log,env=None):
    p=subprocess.run(args,cwd=ROOT,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True,errors='replace',timeout=600,env=env)
    log.parent.mkdir(parents=True,exist_ok=True);log.write_text(p.stdout)
    print(p.stdout,end='',flush=True);need(p.returncode==0,'failed; see '+str(log))


def coverage(result,pe):
    spans={
        'checkpoint1':[(r['rva'],r['size']) for r in read_json(ROOT/'config/manifest.json')['functions']],
        'checkpoint2':[(r['rva'],r['code_size']) for r in read_json(ROOT/'config/checkpoint2/regions.json')['regions']],
        'checkpoint31':[(r['rva'],r['size']) for r in read_json(ROOT/'config/checkpoint31/regions.json')['regions']],
        'checkpoint32':[(r['rva'],r['size']) for r in result['regions']]}
    sets={k:{v for a,n in rows for v in range(a,a+n)} for k,rows in spans.items()}
    before=set.union(*(v for k,v in sets.items() if k!='checkpoint32'));union=before|sets['checkpoint32']
    text=next(s for s in pe.sections if s['name']=='.text')
    need(all(text['rva']<=a<text['rva']+text['virtual_size'] for a in union),'coverage outside .text')
    return dict(previous_code_bytes=len(before),batch_code_bytes=len(sets['checkpoint32']),new_unique_code_bytes=len(union-before),union_code_bytes=len(union),text_bytes=text['virtual_size'],text_percent=100*len(union)/text['virtual_size'],whole_dll_match=False)


def main():
    p=argparse.ArgumentParser();p.add_argument('--prefix',default='cp32');p.add_argument('--skip-build',action='store_true');a=p.parse_args()
    need(re.fullmatch('cp32(?:-[a-z0-9]+)*',a.prefix) is not None,'invalid prefix')
    out=ROOT/'evidence/checkpoint32';out.mkdir(parents=True,exist_ok=True)
    (out/'acceptance.json').write_text('{"result":"IN_PROGRESS"}\n')
    try:
        preflight();prepare();gates=[];results=[];emulated=[]
        sys.path.insert(0,str(ROOT/'tests/checkpoint32'))
        from emulate import exercise
        for suffix,base in [('baseline','0x10000000'),('repeat','0x30000000')]:
            run=a.prefix+'-'+suffix
            if not a.skip_build:
                script='C:\\Mac\\Home\\'+str((ROOT/'tools/build_checkpoint32.ps1').relative_to(Path.home())).replace('/','\\')
                command(['prlctl','exec','Windows 11','powershell.exe','-NoProfile','-ExecutionPolicy','Bypass','-File',script,'-Run',run,'-ImageBase',base],ROOT/'build'/(run+'.log'))
            gate=Gate(run);need(gate.linked.base==int(base,16),'unexpected link base')
            result=gate.verify();emu=exercise(gate)
            gates.append(gate);results.append(result);emulated.append(emu)
            (gate.directory/'verification.json').write_text(json.dumps(result,indent=2)+'\n')
            (gate.directory/'emulation.json').write_text(json.dumps(emu,indent=2)+'\n')
            print('PASS',run,len(result['regions']),'functions;',gate.native_checks,'native checks;',emu['checks'],'emulated cases',flush=True)
        canonical=canonical_objects(gates[0]);need(canonical==canonical_objects(gates[1]),'non-debug code/data/fixups are not reproducible')
        env=os.environ.copy();env['CP32_TEST_RUN']=gates[0].build['run']
        command([sys.executable,'-m','unittest','discover','-s','tests/checkpoint32','-p','test_*.py','-v'],gates[0].directory/'negative-tests.log',env)
        # Existing gates cover the previously accepted bytes; no duplicate retesting of unchanged fixtures.
        for run in ('baseline','repeat'):need(verify(run)['result']=='PASS','CP1 regression failed')
        for run in ('cp2-baseline','cp2-repeat'):need(Checkpoint(run).verify()['result']=='PASS','CP2 regression failed')
        for run in ('cp31-baseline','cp31-repeat'):need(Foundation(run).verify()['result']=='PASS','CP3.1 regression failed')
        measured=coverage(results[0],gates[0].reference)
        report=dict(result='PASS',checkpoint32_status='IN_PROGRESS',batch='ActorPool, ActorPed, Entity and utility first batch',utc=datetime.now(timezone.utc).isoformat(),coverage=measured,regions=results[0]['regions'],data=results[0]['data'],
            builds=[dict(run=g.build['run'],image_base=g.linked.base,native_checks=g.native_checks,emulation=e,build_sha256=sha((g.directory/'build.json').read_bytes()),artifacts=g.build['artifacts']) for g,e in zip(gates,emulated)],
            reproducibility=dict(result='PASS',canonical_objects=canonical,scope='All non-debug COFF sections and symbolic fixups; timestamps and final PE layout excluded'),negative_controls='8 tests, including complete last-byte mutation of each function in both COFF and linked PE',regressions='CP1, CP2 and CP3.1 gates pass',
            limitations=['ActorPool virtual deletion target is not yet integrated; only its full caller is accepted','ActorPed constructor, destructor, scalar deleting destructor, EH and vtables remain pending','Symbolic inline assembly is explicitly labeled; GTA engine targets are outside R5 coverage','This capsule is not a replacement samp.dll; no game deployment or whole-file hash match'])
        (out/'acceptance.json').write_text(json.dumps(report,indent=2)+'\n');(out/'coverage.json').write_text(json.dumps(measured,indent=2)+'\n')
        print(json.dumps(measured,indent=2))
    except Exception as e:
        (out/'acceptance.json').write_text(json.dumps(dict(result='FAIL',checkpoint32_status='IN_PROGRESS',error=str(e)),indent=2)+'\n');raise

if __name__=='__main__':main()
