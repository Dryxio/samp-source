"""Explicit reviewed store aggregation, retaining original full byte acceptance gates."""
import sys,copy
from pathlib import Path
sys.path.insert(0,'tools')
from integration_pipeline import Pipeline,read,write,pinned,require,exclusive,ROOT,owners,sha,local
from external_pipeline import ExternalPipeline
from aggregate_support import aggregate
from external_support import extend
from verify_actor_closure import Gate
class AggregatePipeline(Pipeline):
    run=ExternalPipeline.run
    def __init__(self,*a,**k):
        super().__init__(*a,**k)
        self.extra=read(pinned(self.p['reviewed_externals']))
        self.base=extend(self.base,self.extra)
        self.aggregation=read(pinned(self.p['data_aggregation']))
        self.base=aggregate(self.base,self.aggregation,self.expected)
        self.removed=[r for r in self.aggregation['regions'] if r['old_kind']=='zero']
        self.store_units={r['old_unit'] for r in self.removed}|{r['new_unit'] for r in self.removed}
        for r in self.removed:
            self.owners[r['old_anchor']]=[v for v in self.owners.get(r['old_anchor'],[]) if v['unit']!=r['old_unit']]
    def plan(self):
        sources=self.p.get('sources',[]);root_units={r['unit'] for r in self.roots}
        extras=[s for s in sources if Path(s['target']).stem not in root_units]
        for s in extras:
            target=local(s['target']);pinned(s)
            require(target.parent==ROOT/'client/saco' and target.suffix=='.cpp' and target.stem in self.store_units,'Only reviewed migrated stores can lack code roots')
            require(target.stem in self.old_units,'Cannot introduce unrooted new unit')
            require(sha(target.read_bytes())==s['before_sha256'],'Changed old store source')
        self.p['sources']=[s for s in sources if s not in extras]
        try:return super().plan()
        finally:self.p['sources']=sources
    def audit(self,objects_dir):
        candidates=owners(objects_dir,{})
        for m in self.removed:
            rows=candidates.get(m['old_anchor'],[])
            require(len(rows)==1 and rows[0]['unit']==m['new_unit'],'Store is not uniquely owned by new aggregate')
            require(rows[0]['value']==m['old_bytes_offset_in_new'] and rows[0]['size']==m['new_section_size'],'Source global extent/layout differs')
            require((Path(objects_dir)/(m['old_unit']+'.obj')).exists(),'Old owner not recompiled')
        return super().audit(objects_dir)
    def command(self,stage,argv,timeout=900):
        if stage=='review':
            spec=read(argv[-1]);spec['data_aggregation']=self.p['data_aggregation'];spec['reviewed_externals']=self.p['reviewed_externals'];write(argv[-1],spec)
            argv=[sys.executable,ROOT/'build/overnight/review_aggregate.py',argv[-1]]
        result=super().command(stage,argv,timeout)
        if stage=='review':
            spec=read(argv[-1]);g=Gate(spec['run'],contract_path='config/checkpoint32/'+spec['prefix']+'-contract.json');evidence={}
            for name,e in self.extra.items():
                address=g.external(name,True)
                evidence[name]=dict(linked_va=address,map_providers=g.maps[name],reference=e)
            write(self.out/'new-externals-linked-qualified.json',evidence)
        return result
if __name__=='__main__':
    with exclusive():
        p=AggregatePipeline(sys.argv[2])
        if sys.argv[1]=='audit':print(p.audit(Path(sys.argv[3])))
        else:print(p.run())
