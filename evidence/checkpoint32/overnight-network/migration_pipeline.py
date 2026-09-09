"""Isolated single-owner integration; inherits all normal acceptance checks."""
import sys,copy
from pathlib import Path
sys.path.insert(0,'tools')
from integration_pipeline import Pipeline,read,write,pinned,require,exclusive,ROOT,owners
from migration_support import migrate
class MigrationPipeline(Pipeline):
    def __init__(self,*args,**kwargs):
        super().__init__(*args,**kwargs)
        self.migrations=read(pinned(self.p['owner_migrations']))
        self.original_base=self.base
        self.mapped_base=migrate(self.base,self.migrations,self.expected)
        self.moves=[m for m in self.migrations if m['old_unit']!=m['new_unit']]
        seeds=read(self.seeds_path)
        for m in self.moves:
            if m['old_anchor'] in seeds.get(m['old_unit'],{}):
                rva=seeds[m['old_unit']].pop(m['old_anchor'])
                require(rva==m['rva'],'Migrating wrong seed')
                seeds.setdefault(m['new_unit'],{})[m['new_anchor']]=rva
        self.seeds_path=self.out/'migrated-seeds.json';write(self.seeds_path,seeds)
    def audit(self,objects_dir):
        # Every removed provider must really be absent, and the new provider unique.
        candidate=owners(objects_dir,{})
        for m in self.moves:
            rows=candidate.get(m['new_anchor'],[])
            require(len(rows)==1 and rows[0]['unit']==m['new_unit'],'Migrated provider not unique')
            require((Path(objects_dir)/(m['old_unit']+'.obj')).exists(),'Missing recompiled old owner')
        oldbase,oldowners=self.base,self.owners
        self.base=self.mapped_base
        self.owners=copy.deepcopy(self.owners)
        for m in self.moves:
            self.owners[m['old_anchor']]=[r for r in self.owners.get(m['old_anchor'],[]) if r['unit']!=m['old_unit']]
        try:return super().audit(objects_dir)
        finally:self.base,self.owners=oldbase,oldowners
    def command(self,stage,argv,timeout=900):
        if stage=='review':
            spec=read(argv[-1]);spec['owner_migrations']=self.p['owner_migrations'];write(argv[-1],spec)
            argv=[sys.executable,ROOT/'build/overnight/review_migration.py',argv[-1]]
        return super().command(stage,argv,timeout)
if __name__=='__main__':
    with exclusive():
        p=MigrationPipeline(sys.argv[2])
        if sys.argv[1]=='audit':print(p.audit(Path(sys.argv[3])))
        else:print(p.run())
