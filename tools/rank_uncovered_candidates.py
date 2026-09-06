"""Rank discovery candidates by new code bytes; never grants acceptance."""
import argparse
import json
from pathlib import Path
from binary import need, sha
from verify import ROOT


def accepted_code(proof_path):
    proof = json.loads((ROOT / proof_path).read_text())
    covered = set()
    for file, key, size in (
        ('config/manifest.json', 'functions', 'size'),
        ('config/checkpoint2/regions.json', 'regions', 'code_size'),
        ('config/checkpoint31/regions.json', 'regions', 'size'),
        ('config/checkpoint32/regions.json', 'regions', 'size'),
    ):
        for region in json.loads((ROOT / file).read_text())[key]:
            covered.update(range(region['rva'], region['rva'] + region[size]))
    for path, digest in [*proof.get('previous_proofs', {}).items(), (proof_path, None)]:
        raw = (ROOT / path).read_bytes()
        need(digest is None or sha(raw) == digest, 'previous proof changed: ' + path)
        previous = json.loads(raw)
        need(previous['result'] == 'PASS', 'proof not accepted: ' + path)
        need(sha((ROOT / 'build' / previous['run'] / 'closure.dll').read_bytes()) ==
             previous['artifacts']['closure.dll'], 'accepted artifact changed: ' + path)
        for region in previous['regions']:
            for chunk in region['code_ranges']:
                start = region['rva'] + chunk['offset']
                covered.update(range(start, start + chunk['size']))
    need(len(covered) == proof['coverage']['union_code_bytes'], 'coverage union differs')
    return covered


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--proof', required=True)
    parser.add_argument('--discovery', action='append', required=True)
    parser.add_argument('--output', required=True)
    args = parser.parse_args()
    covered = accepted_code(args.proof)
    inventory = json.loads((ROOT / 'config/checkpoint31/inventory.json').read_text())
    functions = {f['rva']: f for f in inventory['functions']}
    rows = []
    for file in args.discovery:
        for row in json.loads((ROOT / file).read_text())['rows']:
            for candidate in row['candidates']:
                rva = candidate['rva']
                function = functions.get(rva)
                if function is None:
                    continue
                # Count inventory code only, excluding attached switch/EH data.
                code = {address for chunk in function['chunks']
                        for address in range(chunk['rva'], chunk['rva'] + chunk['size'])
                        if rva <= address < rva + row['size']}
                gain = len(code - covered)
                if gain:
                    rows.append(dict(discovery=file, unit=row['unit'], symbol=row['symbol'],
                                     rva=rva, complete_candidate_size=row['size'],
                                     potential_new_code_bytes=gain,
                                     candidate_count=len(row['candidates'])))
    rows.sort(key=lambda row: (-row['potential_new_code_bytes'], row['rva']))
    output = dict(status='DISCOVERY_PRIORITY_ONLY_NOT_ACCEPTED', proof=args.proof,
                  accepted_union_bytes=len(covered), rows=rows)
    (ROOT / args.output).write_text(json.dumps(output, indent=2) + '\n')
    for row in rows[:30]:
        print(row['potential_new_code_bytes'], hex(row['rva']), row['symbol'])


if __name__ == '__main__':
    main()
