"""Validate the existing incremental link once and record its exact coverage."""
import argparse,json,os,subprocess,sys
from pathlib import Path
from datetime import datetime,timezone
from binary import need,sha
from verify import ROOT
from verify_checkpoint31 import read_json
from verify_actor_closure import Gate


def main():
    parser=argparse.ArgumentParser()
    parser.add_argument('--run',default='cp32-closure-linked2')
    parser.add_argument('--contract',default='config/checkpoint32/closure-contract.json')
    parser.add_argument('--report',default='closure-acceptance.json')
    parser.add_argument('--previous',action='append',default=[])
    args=parser.parse_args()
    gate=Gate(args.run,contract_path=args.contract);result=gate.verify()
    sys.path.insert(0,str(ROOT/'tests/checkpoint32'))
    from actor_closure_emulate import exercise
    emulation=exercise(gate)
    test=subprocess.run([sys.executable,'-m','unittest','discover','-s','tests/checkpoint32','-p','test_actor_closure_gate.py','-v'],cwd=ROOT,env=dict(os.environ,ACTOR_GATE_RUN=args.run,ACTOR_GATE_CONTRACT=args.contract),stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True)
    (gate.directory/'negative-controls.log').write_text(test.stdout);need(test.returncode==0,'negative controls failed')
    old=set()
    for file,key,size in [('config/manifest.json','functions','size'),('config/checkpoint2/regions.json','regions','code_size'),('config/checkpoint31/regions.json','regions','size'),('config/checkpoint32/regions.json','regions','size')]:
        for r in read_json(ROOT/file)[key]:old.update(range(r['rva'],r['rva']+r[size]))
    for previous in args.previous:
        proof=read_json(ROOT/previous)
        need(proof['result']=='PASS','previous proof not accepted')
        need(proof['artifacts']['closure.dll']==sha((ROOT/'build'/proof['run']/'closure.dll').read_bytes()),'previous artifact changed')
        for r in proof['regions']:
            for c in r['code_ranges']:old.update(range(r['rva']+c['offset'],r['rva']+c['offset']+c['size']))
    current={v for r in result['regions'] for c in r['code_ranges'] for v in range(r['rva']+c['offset'],r['rva']+c['offset']+c['size'])}
    extent=next(s['virtual_size'] for s in gate.reference.sections if s['name']=='.text')
    coverage=dict(previous_code_bytes=len(old),batch_code_bytes=len(current),new_unique_code_bytes=len(current-old),union_code_bytes=len(old|current),text_bytes=extent,text_percent=100*len(old|current)/extent,whole_dll_match=False)
    inputs={str(p.relative_to(ROOT)):sha(p.read_bytes()) for p in [gate.contract_path,ROOT/'tools/verify_actor_closure.py',ROOT/'tools/draft_actor_contract.py',ROOT/'tools/accept_actor_closure.py',ROOT/'tools/binary.py',ROOT/'tools/verify.py',ROOT/'tools/verify_checkpoint2.py',ROOT/'tools/verify_checkpoint31.py',ROOT/'tests/checkpoint32/actor_closure_emulate.py',ROOT/'tests/checkpoint32/test_actor_closure_gate.py']}
    report=dict(previous_proofs={p:sha((ROOT/p).read_bytes()) for p in args.previous},result='PASS',checkpoint32_status='IN_PROGRESS',utc=datetime.now(timezone.utc).isoformat(),run=gate.directory.name,scope=result['scope'],coverage=coverage,regions=result['regions'],pending=result['pending'],emulation=emulation,negative_controls=dict(result='PASS',tests=6,log_sha256=sha(test.stdout.encode())),link_metadata_sha256=sha((gate.directory/'link.json').read_bytes()),artifacts=gate.link['artifacts'],verification_inputs=inputs,limitations=['One incremental build; no redundant second build at this intermediate batch','AddEntry and PushBack remain unmatched and excluded','ABI exercise follows normal construction/model polling and virtual deletion; actual exception unwinding is not exercised','Original game engine and CRT implementations are outside source coverage','Source coverage is the union of independently certified regions, not a replacement DLL'])
    out=ROOT/'evidence/checkpoint32';(out/args.report).write_text(json.dumps(report,indent=2)+'\n');(out/'coverage-current.json').write_text(json.dumps(coverage,indent=2)+'\n')
    print(json.dumps(coverage,indent=2))

if __name__=='__main__':main()
