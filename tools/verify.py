"""Fail-closed, full-body matching at an explicit R5 address layout."""
import argparse
import json
import struct
from pathlib import Path

from capstone import Cs, CS_ARCH_X86, CS_MODE_32, CS_GRP_JUMP
from capstone.x86 import X86_OP_IMM
from binary import COFF, PE, need, sha, u32

ROOT = Path(__file__).resolve().parent.parent
DECODER = Cs(CS_ARCH_X86,CS_MODE_32)
DECODER.detail = True


def decode(code, address):
    insns = list(DECODER.disasm(code,address))
    need(sum(i.size for i in insns) == len(code), 'undecodable or truncated instructions')
    return insns


def check_boundary(code, address):
    insns = decode(code,address)
    need(insns and insns[-1].mnemonic == 'ret', 'body does not end at final return')
    starts = {i.address for i in insns}
    for i in insns:
        if i.group(CS_GRP_JUMP) and i.operands[0].type == X86_OP_IMM:
            need(i.operands[0].imm in starts, 'branch escapes body or enters instruction middle')


def relocate(obj, fn, manifest, pe, base, raw_override=None, reloc_override=None):
    raw,relocs = obj.function(fn['symbol'])
    if raw_override is not None:
        raw = raw_override
    if reloc_override is not None:
        relocs = reloc_override
    need(len(raw) == fn['size'], 'complete COFF body length mismatch')
    result = bytearray(raw)
    address = base+fn['rva']
    instructions = decode(raw,address)
    expected_abs = {r-fn['rva'] for r in pe.relocations
                    if fn['rva'] <= r < fn['rva']+fn['size']}
    seen_abs,fixups = set(),[]
    for r in relocs:
        off,kind,symbol = r['offset'],r['kind'],r['symbol']
        name = symbol['name']
        need(name in manifest['symbols'], 'unresolved symbol '+name)
        target = manifest['symbols'][name]
        need(0 <= target['rva'] < pe.image_size, 'target outside R5 image')
        inst = next((i for i in instructions if i.address-address <= off < i.address-address+i.size),None)
        need(inst is not None, 'relocation outside decoded instructions')
        local = inst.address-address
        if kind == 20:
            need(inst.mnemonic == 'call' and inst.imm_size == 4 and off == local+inst.imm_offset,
                 'REL32 is not an actual call operand')
            need(target['kind'] == 'unimplemented-function', 'call target is not a declared function')
        elif kind == 6:
            need((inst.disp_size == 4 and off == local+inst.disp_offset) or
                 (inst.imm_size == 4 and off == local+inst.imm_offset),
                 'DIR32 is not a decoded address operand')
            seen_abs.add(off)
        else:
            raise ValueError('unsupported relocation type')
        if target['kind'] == 'constant':
            original = pe.read(target['rva'],target['size'])
            need(obj.defined_data(symbol,target['size']) == original,
                 'compiler constant bytes differ from original')
        else:
            need(symbol['section'] == 0 and symbol['value'] == 0,
                 'expected an explicitly unresolved external declaration')
        addend = u32(raw,off)
        resolved = base+target['rva']+addend
        if kind == 20:
            resolved -= address+off+4
        struct.pack_into('<I',result,off,resolved&0xffffffff)
        fixups.append(dict(offset=off,type='REL32' if kind == 20 else 'DIR32',
                           symbol=name,target_rva=target['rva'],addend=addend))
    need(seen_abs == expected_abs, 'COFF address fixups disagree with original PE relocation sites')
    return bytes(result),fixups


def load(run):
    need(run and all(c.isalnum() or c in '_-' for c in run), 'invalid run identifier')
    manifest = json.loads((ROOT/'config/manifest.json').read_text())
    pe = PE(ROOT/'private/samp.dll',manifest['target_sha256'])
    need(pe.base == manifest['image_base'], 'wrong reference image base')
    build = json.loads((ROOT/'build'/run/'build.json').read_text(encoding='utf-8-sig'))
    pin = json.loads((ROOT/'config/toolchain.json').read_text())
    need(build['toolchain'] == pin['tools'] and build['options'] == pin['options'],
         'toolchain or flags changed')
    objects = {}
    for unit in build['units']:
        name = unit['unit']
        need(name not in objects, 'duplicate compilation unit')
        source = (ROOT/'src'/f'{name}.cpp').read_bytes()
        path = ROOT/'build'/run/f'{name}.obj'
        need(sha(source) == unit['source_sha256'], 'stale build: source changed '+name)
        need(sha(path.read_bytes()) == unit['object_sha256'], 'stale or altered object '+name)
        need(sha((ROOT/'build'/run/'src'/f'{name}.cpp').read_bytes()) == unit['source_sha256'],
             'missing or altered build source snapshot '+name)
        objects[name] = COFF(path)
    need(set(objects) == {f['unit'] for f in manifest['functions']}, 'unit inventory mismatch')
    coverage = set()
    for fn in manifest['functions']:
        rva,size = fn['rva'],fn['size']
        region = set(range(rva,rva+size))
        need(not coverage.intersection(region), 'overlapping function coverage')
        coverage.update(region)
        code = pe.read(rva,size)
        need(sha(code) == fn['sha256'], 'function contract hash changed')
        check_boundary(code,pe.base+rva)
        need(pe.read(rva+size,1) == b'\xcc', 'expected post-function INT3 boundary')
    for target in manifest['symbols'].values():
        if 'original_hex' in target:
            need(pe.read(target['rva'],target['size']).hex() == target['original_hex'],
                 'reference data anchor mismatch')
        if 'entry_sha256_16' in target:
            need(sha(pe.read(target['rva'],16)) == target['entry_sha256_16'],
                 'external function anchor mismatch')
    return manifest,pe,objects,build


def verify(run):
    manifest,pe,objects,build = load(run)
    records = []
    for fn in manifest['functions']:
        record = dict(name=fn['name'],rva=fn['rva'],size=fn['size'],features=fn['features'])
        try:
            obj = objects[fn['unit']]
            code,fixups = relocate(obj,fn,manifest,pe,pe.base)
            expected = pe.read(fn['rva'],fn['size'])
            need(code == expected, 'byte mismatch at offsets '+str(
                [i for i,(a,b) in enumerate(zip(code,expected)) if a != b][:12]))
            check_boundary(code,pe.base+fn['rva'])
            rebased,_ = relocate(obj,fn,manifest,pe,0x30000000)
            need(rebased == pe.rebased(fn['rva'],fn['size'],0x30000000),
                 'rebase mismatch')
            record.update(status='RAW_EXACT' if not fixups else 'RESOLVED_FUNCTION_EXACT',
                          sha256=sha(code),fixups=fixups,rebase_exact=True)
        except ValueError as error:
            record.update(status='FAIL',error=str(error))
        records.append(record)
    matched = [r for r in records if r['status'] != 'FAIL']
    return dict(result='PASS' if len(matched) == len(records) else 'FAIL',
                run=run,target_sha256=manifest['target_sha256'],
                scope='whole function bodies in explicitly resolved original layout; NOT linked DLL',
                functions_matched=len(matched),functions_total=len(records),
                unique_code_bytes=sum(r['size'] for r in matched),
                reference_text_bytes=next(s['virtual_size'] for s in pe.sections if s['name']=='.text'),
                external_dependencies=[n for n,s in manifest['symbols'].items()
                                       if s['kind']=='unimplemented-function'],
                build=build,functions=records)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--run',default='baseline')
    args = parser.parse_args()
    try:
        result = verify(args.run)
    except (ValueError,OSError,KeyError) as error:
        print('FAIL:',error)
        raise SystemExit(1)
    out = ROOT/'build'/args.run/'verification.json'
    out.write_text(json.dumps(result,indent=2)+'\n')
    for fn in result['functions']:
        print(f"{fn['status']:25} {fn['name']:32} {fn['size']:4} bytes {fn.get('error','')}")
    print(f"{result['result']}: {result['functions_matched']}/{result['functions_total']} functions, "
          f"{result['unique_code_bytes']} unique code bytes")
    raise SystemExit(0 if result['result']=='PASS' else 1)


if __name__ == '__main__':
    main()
