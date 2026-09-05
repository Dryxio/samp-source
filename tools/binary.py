"""Small strict PE32/COFF reader for pinned MSVC x86 matching inputs."""
from pathlib import Path
import hashlib
import struct


def sha(data):
    return hashlib.sha256(data).hexdigest()


def need(condition, message):
    if not condition:
        raise ValueError(message)


def u16(data, offset):
    return struct.unpack_from('<H', data, offset)[0]


def u32(data, offset):
    return struct.unpack_from('<I', data, offset)[0]


def section(data, offset):
    size, raw = u32(data, offset+16), u32(data, offset+20)
    flags = u32(data,offset+36)
    uninitialized = raw == 0 and bool(flags & 0x80)
    need(uninitialized or raw+size <= len(data), 'section outside file')
    need(not uninitialized or not flags & 0x20, 'code cannot be uninitialized')
    return dict(name=data[offset:offset+8].rstrip(b'\0').decode('ascii'),
                virtual_size=u32(data, offset+8), rva=u32(data, offset+12),
                size=size, raw=raw, bytes=b'' if uninitialized else data[raw:raw+size],
                uninitialized=uninitialized,
                reloc_ptr=u32(data,offset+24), reloc_count=u16(data,offset+32),
                flags=u32(data,offset+36))


class PE:
    def __init__(self, path, expected_sha=None):
        self.data = Path(path).read_bytes()
        if expected_sha:
            need(sha(self.data) == expected_sha, 'reference SHA-256 mismatch')
        d = self.data
        need(d[:2] == b'MZ', 'not MZ')
        p = u32(d,0x3c)
        need(d[p:p+4] == b'PE\0\0' and u16(d,p+4) == 0x14c, 'not x86 PE')
        opt = p+24
        need(u16(d,opt) == 0x10b, 'not PE32')
        self.base, self.image_size = u32(d,opt+28), u32(d,opt+56)
        self.sections = [section(d,opt+u16(d,p+20)+i*40) for i in range(u16(d,p+6))]
        rva,size = u32(d,opt+96+5*8),u32(d,opt+100+5*8)
        self.relocations = set()
        if size:
            blocks = self.read(rva,size)
            pos = 0
            while pos < size:
                page,n = u32(blocks,pos),u32(blocks,pos+4)
                need(n >= 8 and n%2 == 0 and pos+n <= size, 'bad PE relocation block')
                for off in range(pos+8,pos+n,2):
                    entry = u16(blocks,off)
                    kind,delta = entry>>12,entry&0xfff
                    need(kind in (0,3), 'unsupported PE relocation type')
                    if kind == 3:
                        need(page+delta not in self.relocations, 'duplicate PE relocation')
                        self.relocations.add(page+delta)
                pos += n

    def read(self, rva, length):
        for s in self.sections:
            delta = rva-s['rva']
            if 0 <= delta and delta+length <= s['size']:
                return s['bytes'][delta:delta+length]
        raise ValueError('RVA not backed by file bytes: %#x + %#x' % (rva,length))

    def rebased(self, rva, length, base):
        code = bytearray(self.read(rva,length))
        for fixup in self.relocations:
            if rva <= fixup < rva+length:
                off = fixup-rva
                need(off+4 <= length, 'partial PE relocation in function')
                struct.pack_into('<I',code,off,(u32(code,off)+base-self.base)&0xffffffff)
        return bytes(code)


class COFF:
    def __init__(self, path):
        self.data = d = Path(path).read_bytes()
        need(u16(d,0) == 0x14c and u16(d,16) == 0, 'not ordinary x86 COFF')
        self.sections = [section(d,20+i*40) for i in range(u16(d,2))]
        ptr,count = u32(d,8),u32(d,12)
        strings = ptr+count*18
        need(strings+4 <= len(d), 'missing COFF string table')
        string_end = strings+u32(d,strings)
        need(string_end <= len(d), 'truncated string table')
        self.symbols, self.names = {},{}
        i = 0
        while i < count:
            p = ptr+i*18
            if u32(d,p) == 0:
                start = strings+u32(d,p+4)
                need(strings+4 <= start < string_end, 'bad symbol name offset')
                name = d[start:d.index(b'\0',start,string_end)].decode('ascii')
            else:
                name = d[p:p+8].rstrip(b'\0').decode('ascii')
            symbol = dict(name=name,value=u32(d,p+8),
                          section=struct.unpack_from('<h',d,p+12)[0],
                          type=u16(d,p+14),storage=d[p+16])
            self.symbols[i] = symbol
            self.names.setdefault(name,[]).append(symbol)
            aux = d[p+17]
            need(i+aux < count, 'truncated aux symbols')
            i += 1+aux

    def function(self, name):
        symbols = [s for s in self.names.get(name,[]) if s['section'] > 0 and s['type'] == 0x20]
        need(len(symbols) == 1, 'missing or ambiguous function '+name)
        sym = symbols[0]
        sec = self.sections[sym['section']-1]
        need(sec['name'] == '.text' and sec['flags']&0x20000000, 'not executable .text')
        need(sym['value'] == 0, 'requires whole dedicated function COMDAT')
        occupants = [s for s in self.symbols.values() if s['section'] == sym['section'] and s['type'] == 0x20]
        need(len(occupants) == 1, 'multiple functions in COMDAT')
        relocs,used = [],set()
        for i in range(sec['reloc_count']):
            p = sec['reloc_ptr']+i*10
            off,idx,kind = struct.unpack_from('<IIH',self.data,p)
            need(off+4 <= sec['size'], 'COFF relocation outside function')
            need(kind in (6,20), 'unsupported x86 COFF relocation type')
            need(not used.intersection(range(off,off+4)), 'overlapping COFF relocations')
            used.update(range(off,off+4))
            need(idx in self.symbols, 'relocation references missing/aux symbol')
            relocs.append(dict(offset=off,kind=kind,symbol=self.symbols[idx]))
        return sec['bytes'],relocs

    def defined_data(self, symbol, length):
        need(symbol['section'] > 0, 'symbol not defined in object')
        sec = self.sections[symbol['section']-1]
        need(not sec['uninitialized'], 'uninitialized data requires an explicit zero-fill contract')
        off = symbol['value']
        need(not sec['flags']&0x20000000, 'expected data symbol')
        need(off+length <= sec['size'], 'data symbol out of bounds')
        return sec['bytes'][off:off+length]
