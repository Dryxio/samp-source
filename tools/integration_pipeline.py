"""Fail-closed front end for the existing R5 compiler and acceptance gates.

A proposal pins inputs; it does not certify code. See docs/integration-pipeline.md.
"""
import argparse, contextlib, fcntl, json, re, shutil, struct, subprocess, sys, time
from pathlib import Path
from datetime import datetime, timezone
from binary import COFF, sha
from verify import ROOT
from rank_uncovered_candidates import accepted_code
from index_symbol_owners import owners
from draft_actor_contract import draft

# V1 deliberately supports the existing qualification chain only.
LEGACY_INPUTS = (
    'build/prove_linker_aliases.py',
    'build/qualify_trial30_final_itoa.py',
    'build/qualify_trial30_final_sha1_alias.py',
    'build/agent-independent/qualify_server_bridge_target_alias.py',
    'build/agent-independent/qualify_logger_deleting_aliases.py',
    'build/agent-independent/qualify_replica_network_id_aliases.py',
    'build/qualify_mesh_deleting_alias.py',
    'build/agent-independent/qualify_console_aliases.py',
    'build/agent-independent/qualify_raknet_command_parser_alias.py',
    'build/agent-textdraw/qualify_chat_navigation_alias.py',
    'build/sha1-reviewed-manifest.json',
    'build/agent-independent/replica-network-id-reviewed-manifest.json',
    'build/agent-independent/console-reviewed-manifest.json',
    'build/agent-independent/raknet-command-parser-reviewed-manifest.json',
    'build/agent-textdraw/rw-parser-symbolic-aliases.json',
    'build/associative-binding-controls.json',
    'private/samp.dll',
    'config/checkpoint31/inventory.json',
)

class Rejected(ValueError):
    pass

def require(ok, message):
    if not ok:
        raise Rejected(message)

def read(path):
    return json.loads(Path(path).read_text(encoding='utf-8-sig'))

def write(path, value):
    path = Path(path)
    path.parent.mkdir(parents=True, exist_ok=True)
    temp = path.with_name(path.name + '.tmp')
    temp.write_text(json.dumps(value, indent=2) + '\n')
    temp.replace(path)

def local(path):
    p = (ROOT / path).resolve()
    require(p.is_relative_to(ROOT), 'Path outside repository: ' + str(path))
    return p

def pinned(item):
    p = local(item['path'])
    require(sha(p.read_bytes()) == item['sha256'], 'Changed input: ' + item['path'])
    return p

def section_identity(row):
    return row['section_hash'], row['relocations'], row['value'], row['size']

def collision_errors(baseline, candidate, reachable, newly_selected=()):
    errors = []
    for name in reachable:
        for c in candidate.get(name, []):
            if name not in newly_selected and any(old['unit'] == c['unit'] and section_identity(old) == section_identity(c) for old in baseline.get(name, [])):
                continue
            for old in baseline.get(name, []) + candidate.get(name, []):
                if old['unit'] == c['unit']:
                    continue
                if not (old['comdat'] and c['comdat'] and section_identity(old) == section_identity(c)):
                    errors.append({'symbol': name, 'candidate': c['unit'], 'existing': old['unit'],
                                   'reason': 'Different competing definitions; explicit owner resolution required'})
    return list({(e['symbol'],e['candidate'],e['existing']):e for e in errors}.values())

@contextlib.contextmanager
def exclusive():
    lock = ROOT / 'build/.integration-pipeline.lock'
    lock.parent.mkdir(exist_ok=True)
    with lock.open('a+') as f:
        try:
            fcntl.flock(f, fcntl.LOCK_EX | fcntl.LOCK_NB)
        except BlockingIOError:
            raise Rejected('Another integration pipeline owns the repository/VM lock')
        try:
            yield
        finally:
            fcntl.flock(f, fcntl.LOCK_UN)

class Pipeline:
    def __init__(self, proposal, historical=False):
        self.proposal_path = Path(proposal).resolve()
        self.p = read(self.proposal_path)
        require(self.p.get('version') == 1, 'Unsupported proposal version')
        self.id = self.p['id']
        require(re.fullmatch(r'[a-z][a-z0-9-]{1,60}', self.id), 'Invalid proposal id')
        self.out = ROOT / 'build/integration-pipeline' / self.id
        self.out.mkdir(parents=True, exist_ok=True)
        self.historical = historical
        self.proof_path = pinned(self.p['base']['proof'])
        self.contract_path = pinned(self.p['base']['contract'])
        self.seeds_path = pinned(self.p['base']['seeds'])
        self.proof = read(self.proof_path)
        self.base = read(self.contract_path)
        require(self.contract_path == ROOT/'config/checkpoint32'/(self.p['base']['prefix']+'-contract.json'), 'Base prefix/contract disagree')
        require(self.proof['result'] == 'PASS' and self.base['status'] == 'REVIEWED', 'Unaccepted base')
        require(self.base['run'] == self.proof['run'], 'Proof/contract runs disagree')
        require(self.proof['verification_inputs'].get(str(self.contract_path.relative_to(ROOT))) == sha(self.contract_path.read_bytes()), 'Contract not certified by base proof')
        self.covered = accepted_code(str(self.proof_path.relative_to(ROOT)))
        if not historical:
            require(read(ROOT / 'evidence/checkpoint32/coverage-current.json')['union_code_bytes'] == len(self.covered), 'Stale base: current coverage changed')
        self.base_dir = ROOT / 'build' / self.base['run']
        self.link = read(self.base_dir / 'link.json')
        require(sha((self.base_dir/'link.json').read_bytes()) == self.proof['link_metadata_sha256'], 'Changed link metadata')
        for name, digest in self.proof['artifacts'].items():
            require(sha((self.base_dir/name).read_bytes()) == digest, 'Changed accepted artifact: '+name)
        require(sha((self.base_dir/'closure.dll').read_bytes()) == self.proof['artifacts']['closure.dll'], 'Changed accepted DLL')
        self.profiles = self.link['probes']
        require(len(self.profiles) == 2, 'Expected two existing compiler profiles')
        self.old_units = [r['unit'] for r in read(ROOT/'build'/self.profiles[0]/'probe.json')['units']]
        self.roots = self.p['roots']
        require(self.roots, 'No proposed roots')
        self.regions = {}
        for r in self.base['regions']:
            self.regions.setdefault((r['unit'], r['section']), []).append(r)
        self.owners = owners(self.base_dir, self.regions)
        self.expected = {}
        for item in self.p.get('reviews', []):
            review = read(pinned(item))
            require(review['status'] == 'PASS_FULL_SOURCE_REGIONS_ALL_COFF_PE_TARGETS', 'Unqualified review manifest')
            for r in review['regions']:
                key = (r['unit'], r['rva'], r['size'])
                require(key not in self.expected or self.expected[key] == r, 'Conflicting reviewed region')
                self.expected[key] = r
        self.events = self.out / 'events.jsonl'

    def event(self, stage, **extra):
        with self.events.open('a') as f:
            f.write(json.dumps(dict(utc=datetime.now(timezone.utc).isoformat(), stage=stage, **extra))+'\n')

    def command(self, stage, argv, timeout=900):
        stamp = str(time.time_ns())
        log = self.out / (stage + '-' + stamp + '.log')
        start = time.monotonic()
        with log.open('w') as output:
            try:
                r = subprocess.run([str(x) for x in argv], cwd=ROOT, stdout=output, stderr=subprocess.STDOUT, timeout=timeout)
            except subprocess.TimeoutExpired:
                self.event(stage, seconds=time.monotonic()-start, status='TIMEOUT', log=str(log.relative_to(ROOT)))
                raise Rejected(stage+' timed out; inspect guest processes before retrying')
        self.event(stage, seconds=time.monotonic()-start, exit_code=r.returncode, log=str(log.relative_to(ROOT)))
        require(r.returncode == 0, stage + ' failed; see ' + str(log.relative_to(ROOT)))

    def plan(self):
        inventory = read(ROOT/'config/checkpoint31/inventory.json')
        chunks = {(c['rva'], c['size']) for f in inventory['functions'] for c in f['chunks']}
        unique = set()
        seen = set()
        root_owners = {}
        ob2_units = {x['unit'] for x in read(ROOT/'build'/self.profiles[1]/'probe.json')['units']}
        for r in self.roots:
            require(re.fullmatch(r'[a-zA-Z0-9_]+', r['unit']), 'Only canonical Ob1 units supported in v1')
            require(r['unit'] not in ob2_units, 'Ob2 owner changes require a separate reviewed profile')
            require((r['rva'], r['size']) in chunks, 'Root must be an entire inventoried function chunk')
            key = (r['unit'], r['symbol'])
            require(key not in seen, 'Duplicate proposed root')
            seen.add(key)
            require(r['symbol'] not in root_owners or root_owners[r['symbol']] == r['unit'], 'Competing proposed root owners: '+r['symbol'])
            root_owners[r['symbol']] = r['unit']
            existing = self.owners.get(r['symbol'], [])
            require(not existing or any(x['unit'] == r['unit'] for x in existing), 'Root already implemented under another owner: '+r['symbol'])
            unique.update(range(r['rva'],r['rva']+r['size']))
        require(unique-self.covered, 'Zero unique root bytes: already accepted')
        targets = set()
        for source in self.p.get('sources', []):
            pinned(source)
            target = local(source['target'])
            require(target.parent == ROOT/'client/saco' and target.suffix == '.cpp', 'Only client/saco cpp proposals supported')
            require(target.stem in {r['unit'] for r in self.roots}, 'Source unit has no proposed root')
            require(target not in targets, 'Duplicate source target')
            targets.add(target)
            actual = sha(target.read_bytes()) if target.exists() else None
            require(actual == source.get('before_sha256'), 'Active source changed: '+source['target'])
        self.units = self.old_units + sorted({r['unit'] for r in self.roots}-set(self.old_units))
        plan = dict(status='PLANNED_NOT_ACCEPTED', base_code=len(self.covered), potential_root_bytes=len(unique-self.covered),
                    units=self.units, roots=self.roots, proposal_sha256=sha(self.proposal_path.read_bytes()))
        write(self.out/'plan.json', plan)
        self.event('plan', potential_root_bytes=plan['potential_root_bytes'])
        return plan

    def freeze(self):
        # Proposal sources may be staged outside active code. Check all targets before writing any.
        self.plan()
        units_file=self.out/'units.txt'
        units_file.write_text(','.join(self.units)+'\n')
        pinned_inputs = {str(units_file):sha(units_file.read_bytes())}
        for p in [self.proposal_path, self.proof_path, self.contract_path, self.seeds_path]:
            pinned_inputs[str(p)] = sha(p.read_bytes())
        for p in (ROOT/'tools').rglob('*.py'):
            pinned_inputs[str(p)] = sha(p.read_bytes())
        for p in (ROOT/'tools').rglob('*.ps1'):
            pinned_inputs[str(p)] = sha(p.read_bytes())
        for p in (ROOT/'tests/checkpoint32').glob('*.py'):
            pinned_inputs[str(p)] = sha(p.read_bytes())
        for item in self.p.get('reviews', []) + self.p.get('qualification_inputs', []) + ([self.p['qualification_recipe']] if self.p.get('qualification_recipe') else []):
            p = pinned(item); pinned_inputs[str(p)] = sha(p.read_bytes())
        for source in self.p.get('sources', []):
            p=pinned(source); pinned_inputs[str(p)] = sha(p.read_bytes())
        for source in self.p.get('sources', []):
            target = local(source['target'])
            if target.exists():
                shutil.copyfile(target, self.out/(target.name+'.before'))
            temp = target.with_name(target.name+'.pipeline-tmp')
            shutil.copyfile(pinned(source), temp)
            temp.replace(target)
        for folder in ('client', 'vendor'):
            for p in (ROOT/folder).rglob('*'):
                if p.is_file() and p.suffix.lower() in ('.cpp', '.h', '.hpp', '.inl'):
                    pinned_inputs[str(p)] = sha(p.read_bytes())
        for p in self.base_dir.iterdir():
            if p.is_file(): pinned_inputs[str(p)] = sha(p.read_bytes())
        write(self.out/'frozen-inputs.json', pinned_inputs)
        shutil.copyfile(self.proposal_path, self.out/'proposal.json')

    def unchanged(self):
        for path, digest in read(self.out/'frozen-inputs.json').items():
            require(sha(Path(path).read_bytes()) == digest, 'Frozen input changed: '+path)
        require(read(ROOT/'evidence/checkpoint32/coverage-current.json')['union_code_bytes'] == len(self.covered), 'Coverage changed during integration')

    def audit(self, objects_dir):
        """Check actual emitted closure before linking, including every new local section."""
        audit_start=time.monotonic()
        objects_dir=Path(objects_dir)
        merged=self.out/('audit-'+str(time.time_ns()))
        merged.mkdir()
        for p in self.base_dir.glob('*.obj'):shutil.copyfile(p,merged/p.name)
        for p in objects_dir.glob('*.obj'):shutil.copyfile(p,merged/p.name)
        candidate=owners(objects_dir,{})
        seeds=read(self.seeds_path)
        for r in self.roots:seeds.setdefault(r['unit'],{})[r['symbol']]=r['rva']
        write(self.out/'seeds.json', seeds)
        d=draft(str(merged.relative_to(ROOT/'build')), seeds)
        write(self.out/'draft-prelink.json', d)
        old_pending={(r['unit'],r['rva'],r['size'],r['anchor']) for r in self.base['pending']}
        pending=[r for r in d['pending'] if (r['unit'],r['rva'],r['size'],r['anchor']) not in old_pending]
        old={(r['unit'],r['rva'],r['size']):r for r in self.base['regions']}
        new={(r['unit'],r['rva'],r['size']):r for r in d['regions']}
        missing=[r for k,r in new.items() if k not in old and k not in self.expected]
        review_errors=[]
        for k in new.keys() & self.expected.keys():
            r=new[k]; e=self.expected[k]
            obj=COFF(merged/(r['unit']+'.obj')); sec=obj.sections[r['section']-1]
            actual=[(f['offset'],f['kind'],f['symbol'],(f['reference_va']+struct.unpack_from('<I',sec['bytes'],r['offset']+f['offset'])[0])&0xffffffff) for f in r['fixups']]
            wanted=[(f['site_rva']-r['rva'],f['kind'],f['symbol'],f['target_va']) for f in e['bindings']]
            if r['sha256']!=e['sha256'] or actual!=wanted or r['offset']!=0 or sec['size']!=r['size']:
                review_errors.append(k)
        absent_reviews=[k for k,e in self.expected.items() if k not in new and e['kind']!='.CRT$XCU']
        lost=list(set(old)-set(new))
        changed=[k for k in old.keys() & new.keys() if old[k]['sha256']!=new[k]['sha256'] or
                 [(x['offset'],x['kind'],x['reference_va']) for x in old[k]['fixups']] !=
                 [(x['offset'],x['kind'],x['reference_va']) for x in new[k]['fixups']]]
        reachable={r['anchor'] for r in d['regions']+d['pending']}
        reachable.update(f['symbol'] for r in d['regions'] for f in r.get('fixups', []))
        collisions=collision_errors(self.owners,candidate,reachable,{r['symbol'] for r in self.roots if set(range(r['rva'],r['rva']+r['size']))-self.covered})
        # References to unreviewed imports/CRT/native providers cannot be self-certified by inference.
        externals=[n for n,v in d['externals'].items() if n not in self.base['externals'] or
                   any(v.get(k)!=self.base['externals'][n].get(k) for k in ['kind','reference_va'])]
        report=dict(status='BLOCKED' if pending or missing or lost or changed or collisions or externals or review_errors or absent_reviews else 'CLOSURE_REVIEWED_NOT_ACCEPTED',
                    pending=pending,unreviewed=missing,review_errors=review_errors,absent_reviews=absent_reviews,lost=lost,changed=changed,collisions=collisions,externals=externals)
        write(self.out/'closure-audit.json',report)
        self.event('audit', seconds=time.monotonic()-audit_start, status=report['status'], pending=len(pending), unreviewed=len(missing), collisions=len(collisions))
        require(report['status']!='BLOCKED', 'Emitted closure blocked: '+str(self.out/'closure-audit.json'))
        return report

    def ps(self, script, *args):
        path='C:\\Mac\\Home\\Documents\\GitHub\\samp-r5-matching\\'+str(script).replace('/','\\')
        return ['prlctl','exec','Windows 11','powershell.exe','-NoProfile','-ExecutionPolicy','Bypass','-File',path,*args]

    def run(self):
        require(not self.historical,'Historical mode cannot mutate or accept')
        require(not (self.out/'frozen-inputs.json').exists(),'Run already started; preserve it and use a new proposal id')
        require(self.p.get('qualification_recipe') and self.p.get('qualification_inputs'), 'Pinned legacy qualification recipe required')
        recipe=pinned(self.p['qualification_recipe'])
        require(recipe == ROOT/'build/qualify_trial30b_link.py', 'Unsupported qualification recipe')
        require(set(LEGACY_INPUTS) <= {x['path'] for x in self.p['qualification_inputs']}, 'Missing transitive qualification inputs')
        require(self.p.get('review_note'), 'Semantic provenance/review note required')
        active=subprocess.check_output(['ps','-axo','command'],text=True)
        require(not any('prlctl exec Windows 11' in l or ('powershell.exe' in l and 'samp-r5-matching' in l) for l in active.splitlines()), 'Existing VM command: inspect it before starting')
        self.command('vm-idle',self.ps('tools/check_integration_idle.ps1'),timeout=30)
        self.freeze()
        ob1='cp32-'+self.id+'-ob1';linked='cp32-'+self.id+'-linked1'
        self.command('compile',self.ps('tools/integration/probe_proposal.ps1','-Run',ob1,'-UnitsFile','C:\\Mac\\Home\\Documents\\GitHub\\samp-r5-matching\\'+str((self.out/'units.txt').relative_to(ROOT)).replace('/','\\')))
        self.unchanged()
        self.audit(ROOT/'build'/ob1)
        # Generate exports from the certified base snapshot, never from mutable accumulated exports.
        exports=(self.base_dir/'closure-exports.def').read_text()
        have={x.strip().split()[0] for x in exports.splitlines() if x.strip()}
        for r in self.roots:
            if r['symbol'] not in have:exports+='    '+r['symbol']+'\n';have.add(r['symbol'])
        export_path=self.out/'closure-exports.def';export_path.write_text(exports)
        digest=sha(export_path.read_bytes())
        self.command('link',self.ps('tools/link_actor_closure.ps1','-Run',linked,'-Ob1',ob1,'-Ob2',self.profiles[1],
                     '-ExportsFile','C:\\Mac\\Home\\Documents\\GitHub\\samp-r5-matching\\'+str(export_path.relative_to(ROOT)).replace('/','\\')))
        require(sha(export_path.read_bytes())==digest==sha((ROOT/'build'/linked/'closure-exports.def').read_bytes()),'Export snapshot changed')
        self.unchanged()
        self.command('qualify',[sys.executable,recipe,linked,self.id])
        self.command('spec',[sys.executable,ROOT/'tools/integration/make_spec.py',self.id,self.p['base']['prefix'],str(len(self.covered)),'0'])
        spec_path=ROOT/'build'/(self.id+'-spec.json');spec=read(spec_path)
        spec.update(run=linked,ob1=ob1,ob2=self.profiles[1],manifests=[str(pinned(x).relative_to(ROOT)) for x in self.p['reviews']],
                    draft_file=str((self.out/'draft-linked.json').relative_to(ROOT)),review=self.p['review_note'])
        shutil.copyfile(self.out/'seeds.json',ROOT/'config/checkpoint32'/(self.id+'-seeds.json'))
        write(local(spec['draft_file']),draft(linked,read(self.out/'seeds.json')))
        write(spec_path,spec)
        self.command('review',[sys.executable,ROOT/'tools/integration/review_contract.py',spec_path])
        self.unchanged()
        self.command('accept',[sys.executable,ROOT/'tools/accept_actor_closure.py','--run',linked,'--contract',
                     'config/checkpoint32/'+self.id+'-contract.json','--report',self.id+'-acceptance.json','--previous',str(self.proof_path.relative_to(ROOT))])
        proof=read(ROOT/'evidence/checkpoint32'/(self.id+'-acceptance.json'))
        self.event('accepted',coverage=proof['coverage'])
        write(self.out/'result.json',dict(status='ACCEPTED',proof='evidence/checkpoint32/'+self.id+'-acceptance.json',coverage=proof['coverage']))
        return proof['coverage']

def main():
    a=argparse.ArgumentParser(description=__doc__)
    a.add_argument('stage',choices=['plan','audit','run']);a.add_argument('proposal');a.add_argument('--objects');a.add_argument('--historical',action='store_true')
    args=a.parse_args()
    try:
        with exclusive():
            p=Pipeline(args.proposal,args.historical)
            if args.stage=='plan':result=p.plan()
            elif args.stage=='audit':
                require(args.objects,'--objects required');p.plan();result=p.audit(local(args.objects))
            else:result=p.run()
        print(json.dumps({'status':'OK','stage':args.stage,'report':str(p.out.relative_to(ROOT)),
                          'coverage':result if args.stage=='run' else None}))
    except (Rejected,ValueError,KeyError,FileNotFoundError) as e:
        if 'p' in locals(): p.event('rejected', reason=str(e))
        print(json.dumps({'status':'REJECTED','reason':str(e)}));sys.exit(1)
if __name__=='__main__':main()
