import struct
import sys
import unittest
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[2]/'tools'))
from binary import section


class UninitializedSectionTests(unittest.TestCase):
    def header(self,flags):
        data=bytearray(40);data[:8]=b'.bss\0\0\0\0'
        struct.pack_into('<I',data,16,10000)
        struct.pack_into('<I',data,36,flags)
        return bytes(data)

    def test_bss_has_no_file_bytes(self):
        s=section(self.header(0xc0400080),0)
        self.assertTrue(s['uninitialized']);self.assertEqual(s['size'],10000)
        self.assertEqual(s['bytes'],b'')

    def test_bss_cannot_be_code(self):
        with self.assertRaises(ValueError):section(self.header(0xc04000a0),0)

    def test_initialized_data_must_be_file_backed(self):
        with self.assertRaises(ValueError):section(self.header(0xc0400040),0)
