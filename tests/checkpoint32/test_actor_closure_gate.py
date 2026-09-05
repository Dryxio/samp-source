"""Targeted negative controls for the new EH/data/alias verifier."""
import os,sys,unittest
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[2]/'tools'))
from verify_actor_closure import Gate as ClosureGate

def Gate(**kwargs):
    return ClosureGate(run=os.environ.get("ACTOR_GATE_RUN","cp32-closure-linked2"),contract_path=os.environ.get("ACTOR_GATE_CONTRACT"),**kwargs)

class ClosureGateTests(unittest.TestCase):
    def test_whole_script_section_including_table(self):
        g=Gate(fresh=False);r=next(r for r in g.regions if r['anchor'].startswith('?ScriptCommand@'))
        sec=g.objects[r['unit']].sections[r['section']-1];data=bytearray(sec['bytes']);data[-1]^=1;sec['bytes']=bytes(data)
        with self.assertRaisesRegex(ValueError,'full region differs'):g.verify()
    def test_last_byte_of_typed_command(self):
        g=Gate(fresh=False);r=next(r for r in g.regions if r['kind']=='script-command')
        sec=g.objects[r['unit']].sections[r['section']-1];data=bytearray(sec['bytes']);data[r['offset']+17]^=1;sec['bytes']=bytes(data)
        with self.assertRaisesRegex(ValueError,'full region differs'):g.verify()
    def test_wrong_reviewed_target(self):
        g=Gate(fresh=False);r=next(r for r in g.regions if r['anchor'].startswith('??0CActorPed@'));r['fixups'][0]['reference_va']+=1
        with self.assertRaisesRegex(ValueError,'reviewed target identity'):g.verify()
    def test_linked_vtable_target_changed(self):
        g=Gate(fresh=False);r=next(r for r in g.regions if r['anchor']=='??_7CActorPed@@6B@');at=r['linked_va']-g.linked.base
        sec=next(s for s in g.linked.sections if s['rva']<=at<s['rva']+s['size']);data=bytearray(sec['bytes']);data[at-sec['rva']]^=1;sec['bytes']=bytes(data)
        with self.assertRaisesRegex(ValueError,'full region differs'):g.verify()
    def test_missing_pe_relocation(self):
        g=Gate(fresh=False);r=next(r for r in g.regions if r['anchor']=='??_7CActorPed@@6B@');g.reference.relocations.remove(r['rva'])
        with self.assertRaisesRegex(ValueError,'PE relocation set'):g.verify()
    def test_complete_crt_secondary_chunk(self):
        g=Gate(fresh=False)
        entry=next((e for e in g.contract['externals'].values() if len(e.get('chunks',[]))>1),None)
        if entry is None:self.skipTest('No multi-chunk CRT dependency in this historical contract')
        chunk=entry['chunks'][1];at=chunk['rva']+chunk['size']-1
        sec=next(s for s in g.reference.sections if s['rva']<=at<s['rva']+s['size'])
        data=bytearray(sec['bytes']);data[at-sec['rva']]^=1;sec['bytes']=bytes(data)
        with self.assertRaisesRegex(ValueError,'changed CRT reference chunk'):g.verify()
    def test_nonzero_global(self):
        g=Gate(fresh=False);r=next(r for r in g.regions if r['anchor']=='?pGame@@3PAVCGame@@A');at=r['linked_va']-g.linked.base
        sec=next(s for s in g.linked.sections if s['rva']<=at<s['rva']+max(s['virtual_size'],s['size']))
        # Materialize the loader's zero tail only in the test image.
        data=bytearray(sec['bytes']);data.extend(bytes(max(0,at-sec['rva']+4-len(data))));data[at-sec['rva']]=1;sec['bytes']=bytes(data);sec['size']=len(data)
        with self.assertRaisesRegex(ValueError,'full region differs'):g.verify()

if __name__=='__main__':unittest.main()
