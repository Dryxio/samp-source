"""Evaluate isolated 0.2.5 ports against complete R5 functions.

This is a COFF/reference experiment, not linked-capsule acceptance. It never
updates coverage-current.json. Every relocation is resolved through the explicit
reviewed identity map below; no bytes are masked or truncated.
"""
import json, struct, sys
from pathlib import Path
sys.path.insert(0, str(Path(__file__).resolve().parent))
from binary import COFF, PE, sha, need, u32
from verify import ROOT
from verify_checkpoint2 import import_slots

REFERENCE = 'b72b5dbe725f81864ca3f78bc7063bda56cc05fc7188af822fa7a754432553a2'
# Identities reviewed against the existing game-init contract and R5 call sites.
TARGETS = {
    '?pGame@@3PAVCGame@@A': 0x1026ebac,
    '?pChatWindow@@3PAVCChatWindow@@A': 0x1026eb80,
    '?RequestModel@CGame@@QAEXHH@Z': 0x100a0940,
    '?LoadRequestedModels@CGame@@QAEXXZ': 0x100a0960,
    '?IsModelLoaded@CGame@@QAEHH@Z': 0x100a0970,
    '?AddDebugMessage@CChatWindow@@QAAXPADZZ': 0x100680f0,
    '?Log@CChatWindow@@QAEXHPAD0@Z': 0x100677d0,
    '?PushBack@CChatWindow@@QAEXXZ': 0x10067450,
    '?ModelInfoLoaded@@YGHI@Z': 0x100b45a0,
    '__imp__Sleep@4': 0x100e5254,
    '_time': 0x100ca2cb,
    '_strncpy': 0x100c77f0,
    '??_C@_01JLIPDDHJ@?3?$AA@': 0x100e5ef8,
    '??_C@_0CJ@GGPHJEBP@Warning?3?5Model?5?$CFu?5wouldn?8t?5load?5@': 0x100ec520,
}
FUNCTIONS = {
    'PushBack': (0x67450, 24),
    'AddEntry': (0x67be0, 538),
    'SetModelIndex': (0x9ef50, 217),
}


def resolve(raw, fixups, rva, targets=TARGETS):
    result = bytearray(raw)
    for fix in fixups:
        name = fix['symbol']['name']; need(name in targets, 'Unreviewed target: '+name)
        at = fix['offset']; value = targets[name] + u32(raw, at)
        if fix['kind'] == 20: value -= 0x10000000 + rva + at + 4
        else: need(fix['kind'] == 6, 'Unknown relocation')
        struct.pack_into('<I', result, at, value & 0xffffffff)
    return bytes(result)


def main():
    reference = PE(ROOT/'private/samp.dll', REFERENCE)
    inv = json.loads((ROOT/'config/checkpoint31/inventory.json').read_text())
    for name, (rva, size) in FUNCTIONS.items():
        f = next(f for f in inv['functions'] if f['rva'] == rva)
        need(len(f['chunks']) == 1 and f['chunks'][0]['size'] == size, 'Complete function boundary')
    contract = json.loads((ROOT/'config/checkpoint32/game-init-contract.json').read_text())
    # None of the selected functions is already accepted in this superset.
    need(not {r['rva'] for r in contract['regions']} & {v[0] for v in FUNCTIONS.values()}, 'Already accepted selection')
    for name in ['?pGame@@3PAVCGame@@A','?pChatWindow@@3PAVCChatWindow@@A',
                 '?RequestModel@CGame@@QAEXHH@Z','?LoadRequestedModels@CGame@@QAEXXZ',
                 '?IsModelLoaded@CGame@@QAEHH@Z','?AddDebugMessage@CChatWindow@@QAAXPADZZ',
                 '?Log@CChatWindow@@QAEXHPAD0@Z']:
        need(any(r['anchor'] == name and reference.base+r['rva'] == TARGETS[name] for r in contract['regions']), 'Existing identity changed')
    need(import_slots(reference)[TARGETS['__imp__Sleep@4']] == ('kernel32.dll','Sleep'), 'Sleep IAT identity')
    helper = next(f for f in inv['functions'] if f['rva'] == 0xb45a0)
    need(helper['chunks'] == [{'rva':0xb45a0,'size':40,'sha256':'f5e9939a291e1370f3294dd5458c9137e5c9261932b2b0f0b3b8e08c4767ebf7'}], 'Helper boundary changed')
    need(sha(reference.read(0xb45a0,40)) == helper['chunks'][0]['sha256'], 'Helper body changed')
    rows=[]; controls=[]
    units=['eval_michael_chat_min','eval_michael_chat_r5','eval_michael_model_min','eval_michael_model_r5']
    for profile in ['ob1','ob2']:
        run='cp32-michael-eval-'+profile; directory=ROOT/'build'/run
        probe=json.loads((directory/'probe.json').read_text(encoding='utf-8-sig'))
        for unit in units + (['closure_chat'] if profile == 'ob2' else []):
            source='client/saco/'+unit+'.cpp'
            need(probe['sources'][source] == sha((ROOT/source).read_bytes()), 'Compiled source changed')
            # Verify all headers used by this experiment remain current as well.
            for path, digest in probe['sources'].items():
                if path.endswith('.h'): need(sha((ROOT/path).read_bytes()) == digest, 'Header changed: '+path)
            obj=COFF(directory/(unit+'.obj'))
            for symbol, syms in obj.names.items():
                names=[name for name in FUNCTIONS if symbol.startswith('?'+name+'@')]
                if not names or not any(s['section'] > 0 for s in syms): continue
                name=names[0]; rva,size=FUNCTIONS[name]; raw,fixups=obj.function(symbol)
                resolved=resolve(raw,fixups,rva)
                expected=reference.read(rva,size)
                exact=resolved == expected
                relocation_offsets=sorted(f['offset'] for f in fixups if f['kind']==6)
                pe_offsets=sorted(p-rva for p in reference.relocations if rva<=p<rva+size)
                reloc_exact=relocation_offsets == pe_offsets
                if exact: need(reloc_exact, 'Exact bytes with wrong PE relocation layout')
                # Pooled strings must be full identical source objects.
                for fix in fixups:
                    s=fix['symbol']
                    if s['name'].startswith('??_C@'):
                        sec=obj.sections[s['section']-1]
                        need(s['value']==0 and sec['reloc_count']==0, 'String is not whole section')
                        need(sec['bytes']==reference.read(TARGETS[s['name']]-reference.base,len(sec['bytes'])), 'String differs')
                row=dict(profile=profile,run=run,unit=unit,symbol=symbol,function=name,rva=rva,
                         reference_size=size,source_size=len(raw),source_sha256=sha((ROOT/source).read_bytes()),
                         object_sha256=sha(obj.data),probe_sha256=sha((directory/'probe.json').read_bytes()),
                         reference_sha256=sha(expected),resolved_sha256=sha(resolved),
                         result='EXACT_RESOLVED_COFF_FUNCTION' if exact else 'MISMATCH',
                         pe_relocations_exact=reloc_exact,
                         fixups=[dict(offset=f['offset'],kind=f['kind'],symbol=f['symbol']['name'],reference_va=TARGETS[f['symbol']['name']]) for f in fixups])
                if len(raw)==size:row['difference_offsets']=[i for i in range(size) if resolved[i]!=expected[i]]
                else:row['reason']='COMPLETE_FUNCTION_LENGTH_DIFFERS'
                rows.append(row)
                if exact:
                    changed=bytearray(raw);changed[0]^=1
                    need(resolve(changed,fixups,rva)!=expected,'Byte mutation passed')
                    bad=dict(TARGETS);bad['?ModelInfoLoaded@@YGHI@Z']+=1
                    need(resolve(raw,fixups,rva,bad)!=expected,'Wrong target passed')
                    controls.append(dict(profile=profile,byte_mutation='REJECTED',wrong_helper_target='REJECTED'))
    preserved=json.loads((ROOT/'build/michael-evaluation/preserved-work.json').read_text())
    for p,digest in preserved.items():need(sha((ROOT/p).read_bytes())==digest,'Pre-existing work changed: '+p)
    provenance=json.loads((ROOT/'build/michael-evaluation/reference.json').read_text())
    report=dict(scope='Complete COFF functions resolved to reviewed R5 identities; not a linked capsule acceptance',
                reference_sha256=REFERENCE,reference=provenance,rows=rows,
                exact_unique_functions=sorted({r['function'] for r in rows if r['result']=='EXACT_RESOLVED_COFF_FUNCTION'}),
                exact_unique_function_bytes=sum(FUNCTIONS[n][1] for n in {r['function'] for r in rows if r['result']=='EXACT_RESOLVED_COFF_FUNCTION'}),
                accepted_coverage_increment=0,whole_dll_match=False,negative_controls=controls,
                preexisting_work_hashes=preserved,preexisting_work_preserved=True,
                limitations=['No linked-capsule gate or runtime ABI exercise for the new model function yet.',
                'ModelInfoLoaded at B45A0 is identified and hash pinned, but its implementation and A7A40 dependency remain unmatched and are excluded.',
                'SetModelIndex retains the symbolic inline assembly already present in the 0.2.5 source; it is not a pure C++ result.',
                'EvaluationEntity adds no fields or virtual methods; this isolated experiment reuses the existing packed R5 CEntity layout.',
                'Three targeted functions are a small sample of existing gaps, not a representative estimate for the whole DLL.'])
    (ROOT/'evidence/checkpoint32/michael-source-evaluation.json').write_text(json.dumps(report,indent=2)+'\n')
    print(json.dumps({k:report[k] for k in ['exact_unique_functions','exact_unique_function_bytes','accepted_coverage_increment','preexisting_work_preserved']},indent=2))

if __name__ == '__main__':main()
