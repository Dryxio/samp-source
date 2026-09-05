"""Mutation tests exercise actual full-code and symbol-target failures."""
import os,sys,unittest
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[2]/'tools'))
from verify_checkpoint32 import Gate
from binary import COFF
from verify_checkpoint2 import symbol_address

class GateTests(unittest.TestCase):
    def gate(self):return Gate(os.environ.get('CP32_TEST_RUN','cp32-baseline'),False)
    def test_valid(self):self.assertEqual(self.gate().verify()['result'],'PASS')
    def test_last_byte_every_function(self):
        for r in self.gate().contract['regions']:
            with self.subTest(symbol=r['symbol']):
                g=self.gate();o=g.objects[r['unit']+'.obj'];sym=next(s for s in o.names[r['symbol']] if s['section']>0)
                sec=o.sections[sym['section']-1];b=bytearray(sec['bytes']);b[-1]^=1;sec['bytes']=bytes(b)
                with self.assertRaisesRegex(ValueError,'full bytes differ'):g.verify()
    def test_linked_last_byte_every_function(self):
        for r in self.gate().contract['regions']:
            with self.subTest(symbol=r['symbol']):
                g=self.gate();rva=symbol_address(g.maps,r['symbol'])-g.linked.base+r['size']-1
                sec=next(s for s in g.linked.sections if s['rva']<=rva<s['rva']+s['size'])
                b=bytearray(sec['bytes']);b[rva-sec['rva']]^=1;sec['bytes']=bytes(b)
                with self.assertRaisesRegex(ValueError,'full bytes differ'):g.verify()
    def test_wrong_call_target(self):
        g=self.gate();g.contract['regions'][0]['rva']+=16
        with self.assertRaises(ValueError):g.verify()
    def test_missing_dependency(self):
        g=self.gate();g.contract['regions']=[r for r in g.contract['regions'] if 'GamePool_Ped_GetAt@' not in r['symbol']]
        with self.assertRaisesRegex(ValueError,'uncontracted dependency'):g.verify()
    def test_truncated_function(self):
        g=self.gate();g.contract['regions'][0]['size']-=1
        with self.assertRaisesRegex(ValueError,'complete section boundary'):g.verify()
    def test_missing_pe_relocation(self):
        g=self.gate();r=next(r for r in g.contract['regions'] if 'GetHealth@' in r['symbol'])
        g.reference.relocations.remove(r['rva']+9)
        with self.assertRaisesRegex(ValueError,'PE fixup set'):g.verify()
    def test_changed_constant(self):
        g=self.gate();r=g.contract['data'][0];o=g.objects[r['unit']+'.obj']
        sym=next(s for s in o.names[r['symbol']] if s['section']>0);o.sections[sym['section']-1]['bytes']=b'\x01\x00\x00\x00'
        with self.assertRaisesRegex(ValueError,'full bytes differ'):g.verify()

if __name__=='__main__':unittest.main()
