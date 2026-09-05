"""Adversarial controls for the acceptance gate, using real compiled fixtures."""
import copy
import sys
import unittest
from pathlib import Path

sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from binary import need
from verify import load, relocate, check_boundary


class AcceptanceGateTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.manifest,cls.pe,cls.objects,_ = load('baseline')

    def accept(self, fn, **kwargs):
        manifest = kwargs.pop('manifest',self.manifest)
        code,_ = relocate(self.objects[fn['unit']],fn,manifest,self.pe,self.pe.base,**kwargs)
        need(code == self.pe.read(fn['rva'],fn['size']), 'not byte exact')
        check_boundary(code,self.pe.base+fn['rva'])

    def test_all_last_byte_mutations_rejected(self):
        for fn in self.manifest['functions']:
            with self.subTest(fn=fn['name']):
                raw,_ = self.objects[fn['unit']].function(fn['symbol'])
                bad = bytearray(raw); bad[-1] ^= 1
                with self.assertRaises(ValueError):
                    self.accept(fn,raw_override=bad)

    def test_truncated_and_extended_body_rejected(self):
        for fn in self.manifest['functions']:
            raw,_ = self.objects[fn['unit']].function(fn['symbol'])
            for bad in (raw[:-1],raw+b'\xcc'):
                with self.subTest(fn=fn['name'],length=len(bad)):
                    with self.assertRaises(ValueError):
                        self.accept(fn,raw_override=bad)

    def test_wrong_targets_rejected_for_every_relocation(self):
        for fn in self.manifest['functions']:
            _,relocs = self.objects[fn['unit']].function(fn['symbol'])
            for rel in relocs:
                with self.subTest(fn=fn['name'],symbol=rel['symbol']['name']):
                    changed = copy.deepcopy(self.manifest)
                    changed['symbols'][rel['symbol']['name']]['rva'] += 1
                    with self.assertRaises(ValueError):
                        self.accept(fn,manifest=changed)

    def test_unsupported_and_missing_relocations_rejected(self):
        fn = next(f for f in self.manifest['functions'] if f['name']=='StoreLocalCamera')
        _,relocs = self.objects[fn['unit']].function(fn['symbol'])
        for replacement in ([],[dict(r,kind=999) for r in relocs]):
            with self.assertRaises(ValueError):
                self.accept(fn,reloc_override=replacement)

    def test_unknown_symbol_rejected(self):
        fn = next(f for f in self.manifest['functions'] if f['name']=='Game.SetGravity')
        changed = copy.deepcopy(self.manifest)
        del changed['symbols']['_Unprotect']
        with self.assertRaises(ValueError):
            self.accept(fn,manifest=changed)

    def test_relocation_moved_into_opcode_rejected(self):
        fn = next(f for f in self.manifest['functions'] if f['name']=='ActorPool.DeleteAll')
        _,relocs = self.objects[fn['unit']].function(fn['symbol'])
        changed = copy.deepcopy(relocs); changed[0]['offset'] -= 1
        with self.assertRaises(ValueError):
            self.accept(fn,reloc_override=changed)

    def test_branch_outside_contract_rejected(self):
        fn = next(f for f in self.manifest['functions'] if f['name']=='LocalPlayer.AreKeysChanged')
        code = self.pe.read(fn['rva'],fn['size'])
        # Old comparator checked only 47 bytes, omitting the nonzero return path.
        with self.assertRaises(ValueError):
            check_boundary(code[:47],self.pe.base+fn['rva'])


if __name__ == '__main__':
    unittest.main()
