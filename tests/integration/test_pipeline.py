"""Historical artifact regressions; require retained local build snapshots."""
import json
import sys
import unittest
from pathlib import Path
sys.path.insert(0, str(Path(__file__).resolve().parents[2]/'tools'))
from integration_pipeline import Pipeline, Rejected, ROOT, collision_errors, exclusive, pinned, sha, write


def pin(path):
    return dict(path=path, sha256=sha((ROOT/path).read_bytes()))


def proposal(identifier, prefix, roots, reviews=()):
    p = dict(version=1, id=identifier, base=dict(prefix=prefix,
        proof=pin('evidence/checkpoint32/'+prefix+'-acceptance.json'),
        contract=pin('config/checkpoint32/'+prefix+'-contract.json'),
        seeds=pin('config/checkpoint32/'+prefix+'-seeds.json')),
        roots=roots, reviews=[pin(x) for x in reviews])
    file=ROOT/'build/integration-pipeline'/('test-'+identifier+'.json')
    write(file,p)
    return file


class HistoricalPipeline(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.spec=json.loads((ROOT/'build/trial30f-three-spec.json').read_text())
        cls.roots=[r for f in cls.spec['seed_files'] for r in json.loads((ROOT/f).read_text())]

    def test_positive_historical_lot(self):
        p=Pipeline(proposal('test-positive','trial30f-two',self.roots,self.spec['manifests']),historical=True)
        self.assertEqual(p.plan()['potential_root_bytes'],915)
        self.assertEqual(p.audit(ROOT/'build/cp32-trial30f-three-ob1b')['status'],'CLOSURE_REVIEWED_NOT_ACCEPTED')
        with self.assertRaisesRegex(Rejected,'Historical mode'):
            p.run()

    def test_already_accepted_roots(self):
        p=Pipeline(proposal('test-duplicate','trial30f-three',self.roots))
        with self.assertRaisesRegex(Rejected,'Zero unique'):
            p.plan()

    def test_stale_base(self):
        with self.assertRaisesRegex(Rejected,'Stale base'):
            Pipeline(proposal('test-stale','trial30f-two',self.roots))

    def test_router_closure_rejected_before_link(self):
        roots=json.loads((ROOT/'build/trial30f/network-agent/router-send-seeds.json').read_text())
        p=Pipeline(proposal('test-router','trial30f-three',roots,['build/trial30f/network-agent/router-send-reviewed-manifest.json']))
        p.plan()
        with self.assertRaisesRegex(Rejected,'Emitted closure blocked'):
            p.audit(ROOT/'build/cp32-trial30f-ninth1')
        report=json.loads((p.out/'closure-audit.json').read_text())
        self.assertEqual(len(report['pending']),9)
        self.assertEqual(len(report['unreviewed']),95)
        self.assertFalse((ROOT/'build/cp32-test-router-linked1').exists())

    def test_actual_competing_comdat(self):
        p=Pipeline(proposal('test-comdat','trial30f-three',self.roots))
        name='??_EPlayerIdAndGroupId@ConnectionGraph@@QAEPAXI@Z'
        self.assertIn(77,{r['size'] for r in p.owners[name]})
        self.assertIn(84,{r['size'] for r in p.owners[name]})
        self.assertTrue(collision_errors(p.owners,p.owners,{name},{name}))

    def test_review_binding_tamper(self):
        p=Pipeline(proposal('test-review-tamper','trial30f-two',self.roots,self.spec['manifests']),historical=True)
        region=next(r for r in p.expected.values() if r['bindings'])
        region['bindings'][0]['target_va'] ^= 4
        p.plan()
        with self.assertRaisesRegex(Rejected,'Emitted closure blocked'):
            p.audit(ROOT/'build/cp32-trial30f-three-ob1b')
        self.assertTrue(json.loads((p.out/'closure-audit.json').read_text())['review_errors'])

    def test_source_precondition(self):
        p=Pipeline(proposal('test-source-race','trial30f-two',self.roots,self.spec['manifests']),historical=True)
        name='client/saco/'+self.roots[0]['unit']+'.cpp'
        p.p['sources']=[dict(pin(name),target=name,before_sha256='0'*64)]
        with self.assertRaisesRegex(Rejected,'Active source changed'):
            p.plan()

    def test_lock_rejects_second_integrator(self):
        with exclusive():
            with self.assertRaisesRegex(Rejected,'owns'):
                with exclusive():
                    self.fail('Acquired twice')

    def test_changed_pinned_input(self):
        with self.assertRaisesRegex(Rejected,'Changed input'):
            pinned(dict(path='AGENTS.md',sha256='0'*64))

    def test_new_duplicate_owner(self):
        row=dict(unit='existing',comdat=False,section_hash='a',relocations=[],value=0,size=5)
        new=dict(row,unit='candidate')
        self.assertTrue(collision_errors({'symbol':[row]},{'symbol':[new]},{'symbol'}))

    def test_two_new_competing_owners(self):
        row=dict(unit='first',comdat=False,section_hash='a',relocations=[],value=0,size=5)
        self.assertTrue(collision_errors({}, {'symbol':[row,dict(row,unit='second')]}, {'symbol'}))

if __name__=='__main__':
    unittest.main()
