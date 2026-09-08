import hashlib
import json
import struct
import sys
from pathlib import Path

sys.path.insert(0, "tools")
from binary import COFF, PE, sha, u32
from rank_uncovered_candidates import accepted_code
from verify_checkpoint2 import relocs
from verify_actor_closure import initial_bytes

BASELINE = "evidence/checkpoint32/trial30d-two-acceptance.json"
RUN = Path("build/cp32-trial30e-network1")
UNIT = "closure_vendor_networktypes_helpers"
OBJ = RUN / (UNIT + ".obj")
SRC = RUN / "client/saco/closure_vendor_networktypes_helpers.cpp"
OUTDIR = Path("build/trial30e/network-agent")

pe = PE(Path("private/samp.dll"))
coff = COFF(OBJ)
contract = json.loads(Path("config/checkpoint32/trial30d-two-contract.json").read_text())
providers = {r["anchor"]: pe.base + r["rva"] + r.get("anchor_offset", 0)
             for r in contract["regions"]}
providers.update({name: ext["reference_va"] for name, ext in contract["externals"].items()})
providers["__except_list"] = 0

plan = [
    ("?my_itoa@@YAPADHPADH@Z", 210544, 110),
    ("?SetBinaryAddress@PlayerID@@QAEXPBD@Z", 210976, 20),
    ("?SetPeerToPeerMode@NetworkID@@SAX_N@Z", 211408, 10),
]

todo = []
seeds = []
sections = {}
for name, rva, size in plan:
    sym = next(s for s in coff.names[name] if s["section"] > 0 and s["type"] == 32)
    sec = coff.sections[sym["section"] - 1]
    assert sec["size"] == size, (name, sec["size"], size)
    providers[name] = pe.base + rva
    todo.append((sym["section"], rva))
    seeds.append(dict(unit=UNIT, symbol=name, rva=rva, size=size))

ids = {}
seen = {}
bindings_by_section = {}
while todo:
    section_index, rva = todo.pop()
    if section_index in seen:
        assert seen[section_index] == rva, (section_index, seen[section_index], rva)
        continue
    seen[section_index] = rva
    sec = coff.sections[section_index - 1]
    ref = initial_bytes(pe, rva, sec["size"])
    raw = bytearray(sec["bytes"])
    bindings = []
    for f in relocs(coff, section_index):
        off, sym, kind = f["offset"], f["symbol"], f["kind"]
        name = sym["name"]
        stored = u32(raw, off)
        target = u32(ref, off)
        if kind == 20:
            target = (target + pe.base + rva + off + 4) & 0xFFFFFFFF
        if sym["section"] > 0:
            target_rva = target - pe.base - sym["value"]
            todo.append((sym["section"], target_rva))
            ids[name] = target
        else:
            assert providers.get(name) == target, (name, hex(target), hex(providers.get(name, 0)))
            ids[name] = target
        bindings.append(dict(symbol=name, site_rva=rva + off, kind=kind, target_va=target))
        if kind == 20:
            value = (target - (pe.base + rva + off + 4)) & 0xFFFFFFFF
        else:
            assert kind == 6
            value = target
        struct.pack_into("<I", raw, off, value)
    assert bytes(raw) == ref, (section_index, hex(rva))
    pe_relocs = {x for x in pe.relocations if rva <= x < rva + sec["size"]}
    obj_relocs = {b["site_rva"] for b in bindings if b["kind"] == 6 and b["symbol"] != "__except_list"}
    assert pe_relocs == obj_relocs, (section_index, hex(rva), pe_relocs, obj_relocs)
    bindings_by_section[section_index] = bindings
    sections[section_index] = rva

regions = []
for section_index, rva in sorted(sections.items()):
    sec = coff.sections[section_index - 1]
    anchor = next(s for s in coff.symbols.values()
                  if s["section"] == section_index and s["name"] != sec["name"])
    kind = "code" if sec["flags"] & 0x20 else "data"
    regions.append(dict(
        unit=UNIT,
        section=section_index,
        anchor=anchor["name"],
        anchor_offset=anchor["value"],
        rva=rva,
        size=sec["size"],
        kind=kind,
        sha256=sha(initial_bytes(pe, rva, sec["size"])),
        bindings=bindings_by_section[section_index],
    ))

covered = accepted_code(BASELINE)
physical_code = {a for r in regions if r["kind"] == "code"
                 for a in range(r["rva"], r["rva"] + r["size"])}
new_unique = physical_code - covered
assert len(new_unique) == 140, len(new_unique)

manifest = dict(
    status="PASS_FULL_SOURCE_REGIONS_ALL_COFF_PE_TARGETS",
    run=RUN.name,
    baseline=len(covered),
    new_unique_code=len(new_unique),
    already_covered_code=len(physical_code & covered),
    regions=regions,
    seeds=seeds,
    bindings_checked=sum(len(r["bindings"]) for r in regions),
    source025_direct=0,
    object_sha256={UNIT: sha(OBJ.read_bytes())},
    source_sha256={UNIT: sha(SRC.read_bytes())},
    external_targets={name: va for name, va in ids.items() if name not in [x["symbol"] for x in seeds]},
    literal_extent="0123456789abcdef\\0 is 17 bytes including NUL",
    crt_initialization="object .drectve requests LIBCMT/OLDNAMES; no CRT initializer section is in the selected closure",
)
(OUTDIR / "networktypes-reviewed-manifest.json").write_text(json.dumps(manifest, indent=2) + "\n")
(OUTDIR / "networktypes-seeds.json").write_text(json.dumps(seeds, indent=2) + "\n")
print("PASS", len(regions), "regions", manifest["bindings_checked"], "bindings", len(new_unique), "new code")
