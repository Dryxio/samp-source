"""Controls for archive integrity, full raw spans, linkage and honest coverage."""
import sys
import unittest
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[2]/'tools'))
from verify_checkpoint31 import Gate,verify_region,archive_members
from verify_checkpoint2 import symbol_address


class FoundationGateTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls): cls.g=Gate('cp31-baseline')

    def check_region(self,contract,obj=None,address=None):
        return verify_region(contract,obj or self.g.objects[contract['object']],
                             self.g.reference,self.g.linked,
                             address or symbol_address(self.g.maps,contract['symbol']))

    def test_all_linked_regions_pass(self): self.assertEqual(self.g.verify()['result'],'PASS')

    def test_each_last_byte_mutation_is_rejected(self):
        for contract in self.g.contract['regions']:
            with self.subTest(symbol=contract['symbol']):
                obj=self.g.objects[contract['object']]
                symbol=next(s for s in obj.names[contract['symbol']] if s['section']>0 and s['type']==32)
                section=obj.sections[symbol['section']-1];original=section['bytes']
                changed=bytearray(original);changed[-1]^=1;section['bytes']=bytes(changed)
                try:
                    with self.assertRaises(ValueError):self.check_region(contract)
                finally:section['bytes']=original

    def test_truncated_and_extended_spans_rejected(self):
        for delta in (-1,1):
            contract=dict(self.g.contract['regions'][0]);contract['size']+=delta
            with self.assertRaises(ValueError):self.check_region(contract)

    def test_wrong_linked_address_rejected(self):
        contract=self.g.contract['regions'][0]
        with self.assertRaises(ValueError):self.check_region(contract,address=symbol_address(self.g.maps,contract['symbol'])+1)

    def test_unknown_symbol_rejected(self):
        contract=dict(self.g.contract['regions'][0],symbol='not_a_real_symbol')
        with self.assertRaises(ValueError):self.check_region(contract,address=self.g.linked.base+0x1000)

    def test_changed_reference_hash_rejected(self):
        contract=dict(self.g.contract['regions'][0],sha256='0'*64)
        with self.assertRaises(ValueError):self.check_region(contract)

    def test_hidden_pe_relocation_rejected(self):
        contract=self.g.contract['regions'][0];site=contract['rva']
        self.g.reference.relocations.add(site)
        try:
            with self.assertRaises(ValueError):self.check_region(contract)
        finally:self.g.reference.relocations.remove(site)

    def test_archive_truncation_rejected(self):
        raw=(self.g.directory/'raknet.lib').read_bytes()
        with self.assertRaises(ValueError):archive_members(raw[:-5])

    def test_source_inventory_cannot_omit_a_unit(self):
        units=self.g.build['units'];self.g.build['units']=units[:-1]
        try:
            with self.assertRaises(ValueError):self.g.fresh()
        finally:self.g.build['units']=units


if __name__=='__main__':unittest.main()
