"""Mutations targeting the new linker, table, FS and unwind acceptance surfaces."""
import sys
import unittest
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[2]/'tools'))
from verify_checkpoint2 import Checkpoint


class LinkedGateTests(unittest.TestCase):
    def setUp(self): self.c=Checkpoint('cp2-baseline')

    def region(self,anchor,unit=None):
        return next(r for r in self.c.regions if r['anchor']==anchor and (not unit or r['unit']==unit))

    def test_full_linked_image_passes(self): self.assertEqual(self.c.verify()['result'],'PASS')

    def test_last_jump_table_entry_corruption_rejected(self):
        r=self.region('_FormatBits');b=bytearray(r['raw']);b[-4]=1;r['raw']=bytes(b)
        with self.assertRaises(ValueError): self.c.verify()

    def test_unwind_state_metadata_corruption_rejected(self):
        r=self.region('$T333','buffer');b=bytearray(r['raw']);b[12]=2;r['raw']=bytes(b)
        with self.assertRaises(ValueError): self.c.verify()

    def test_wrong_source_call_target_rejected(self):
        r=self.region('?DeleteAll@VirtualPool@@QAEXXZ')
        r['relocs'][0]['symbol']=dict(self.c.objects['capsule'].names['?UpdateCount@VirtualPool@@QAEXXZ'][0])
        with self.assertRaises(ValueError): self.c.verify()

    def test_wrong_generated_unwind_target_rejected(self):
        r=self.region('$T333','buffer')
        r['relocs'][0]['symbol']=dict(r['relocs'][0]['symbol'],value=8)
        with self.assertRaises(ValueError): self.c.verify()

    def test_fs_pseudo_symbol_cannot_be_arbitrary_constant(self):
        self.c.manifest['externals']['__except_list']['address']=4
        with self.assertRaises(ValueError): self.c.verify()

    def test_iat_name_mismatch_rejected(self):
        self.c.manifest['externals']['__imp__VirtualProtect@16']['import']='VirtualAlloc'
        with self.assertRaises(ValueError): self.c.verify()

    def test_linked_byte_mutation_rejected(self):
        r=self.region('_Unprotect');rva=r['linked_va']-self.c.linked.base
        sec=next(s for s in self.c.linked.sections if s['rva']<=rva<s['rva']+s['size'])
        b=bytearray(sec['bytes']);b[rva-sec['rva']+r['size']-1]^=1;sec['bytes']=bytes(b)
        with self.assertRaises(ValueError): self.c.verify()

    def test_moved_linked_section_rejected(self):
        self.region('$T333','layer')['linked_va']+=4
        with self.assertRaises(ValueError): self.c.verify()


if __name__=='__main__': unittest.main()
