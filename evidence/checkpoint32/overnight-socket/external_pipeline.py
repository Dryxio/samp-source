"""Pinned explicit runtime additions; all source/link/negative/ABI gates unchanged."""
import sys
from pathlib import Path
sys.path.insert(0,'tools')
from integration_pipeline import Pipeline,pinned,read,write,exclusive,ROOT,require,LEGACY_INPUTS,draft,sha,local
import subprocess,shutil
from external_support import extend
from verify_actor_closure import Gate
from binary import sha
class ExternalPipeline(Pipeline):
    def __init__(self,*a,**k):
        super().__init__(*a,**k)
        self.extra=read(pinned(self.p['reviewed_externals']))
        self.base=extend(self.base,self.extra)
    def command(self,stage,argv,timeout=900):
        if stage=='review':
            spec=read(argv[-1]);spec['reviewed_externals']=self.p['reviewed_externals'];write(argv[-1],spec)
            argv=[sys.executable,ROOT/'build/overnight/review_externals.py',argv[-1]]
        result=super().command(stage,argv,timeout)
        if stage=='review':
            spec=read(argv[-1]);g=Gate(spec['run'],contract_path='config/checkpoint32/'+spec['prefix']+'-contract.json');evidence={}
            for name,e in self.extra.items():
                address=g.external(name,True)
                if e['kind']=='crt':require(sha(g.linked.read(address-g.linked.base,e['size']))==e['sha256'],'Linked CRT full bytes differ')
                evidence[name]=dict(linked_va=address,map_providers=g.maps[name],reference=e)
            write(self.out/'new-externals-linked-qualified.json',evidence)
        return result
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
            if r['symbol'] not in have:
                symbol=r['symbol']
                spelling=symbol[1:] if symbol.startswith('_') and '@' not in symbol else symbol
                exports+='    '+spelling+'\n';have.add(symbol)
                self.event('export-spelling', coff_symbol=symbol, def_export=spelling)
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

if __name__=='__main__':
    with exclusive():
        p=ExternalPipeline(sys.argv[2])
        if sys.argv[1]=='audit':print(p.audit(Path(sys.argv[3])))
        else:print(p.run())
