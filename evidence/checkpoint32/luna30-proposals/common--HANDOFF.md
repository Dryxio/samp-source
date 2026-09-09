# Luna30 common / GUI-DXUT handoff

Status: PASS object-root handoff. No active source tree was edited, no VM or
new compilation was run, and no math replacement was compiled or installed.

The pipeline-compatible PASS manifest is
[luna30-pass-manifest.json](luna30-pass-manifest.json); the required root list
is [luna30-roots.json](luna30-roots.json). `owners-current.json` records the
actual COFF owners from `tools/index_symbol_owners.py`.

The manifest compares complete reference roots, resolves every COFF relocation
to an absolute R5 PE target, and reports no masked bytes. GUI uses the freshly
compiled root object in `cp32-luna30-candidates1`; math uses the already
accepted `cp32-pipeline-next-two-linked1/closure_dxut_direction.obj` section 18.
The math source proposal remains preserved for evidence but is not a provider.

Baseline and deduplication

- Proof: `evidence/checkpoint32/pipeline-next-two-acceptance.json`, PASS,
  run `cp32-pipeline-next-two-linked1`, accepted union `428758` code bytes.
- Contract: `config/checkpoint32/pipeline-next-two-contract.json`.
- Ranking was run with `tools/rank_uncovered_candidates.py`; its output is
  `rank-scope.json`. The owner inventory from
  `tools/index_symbol_owners.py` is `owners-baseline.json`.
- Duplicate `GetDXUTState`, string helpers, and ListBox candidates were not
  proposed. EditBox/IMEEditBox constructors were also excluded: their vtable
  and destructor closure is not closed, and ListBox constructor/render remains
  a known failed family.
- Handoff artifact hashes: `rank-scope.json`
  `715ec83f826d2583fc25faed183e106f9143d4b958c77aaa96f437944c9bf579`,
  `owners-baseline.json`
  `eb26825193002f27a4c8c0eba687bc7d3904cab55e249b3ac091eaa9654714d7`.

## Proposal A: GUI resource-manager message root

Source: [proposal_gui_resource_msgproc.cpp](proposal_gui_resource_msgproc.cpp)

Proposal source SHA-256:
`031da1df7de4caa5636c6ae8636192d91698d781e5861cd24aa56a042a96d298`.

| unit | symbol | original RVA | whole size | candidate source/object |
|---|---|---:|---:|---|
| `closure_gui_resource_msgproc` | `?MsgProc@CDXUTDialogResourceManager@@QAE_NPAUHWND__@@IIJ@Z` | `0x8CE50` (577104) | 31 | `client/saco/eval_gui_full.cpp:307-317`; object `eval_gui_full.obj` |

Candidate object SHA-256 is
`13a749cb460dd046a976ac4a34cb4fe264a336055699a41a5eb0b2c7114f0250`.
The dedicated `.text` COMDAT is 31 bytes, COFF storage class 2, with one
`REL32` (kind 20) at offset 16. Its only target is:

- `?StaticMsgProc@CDXUTIMEEditBox@@SA_NIIJ@Z`: actual accepted owner
  `closure_gui_ime`, `.text` COMDAT section 94, RVA `0x8C4C0`
  (575552), size 659. This is a real existing provider, not a declaration
  alias or new IME implementation.

The source body hash for the pinned lines is
`4306cd8832c8f6d914fcff19bf455906229c31c40f118c8ebfb224a97535e7cf`.

## Proposal B: one closed DXUT math root

Source: [proposal_dxut_matrix_operator.cpp](proposal_dxut_matrix_operator.cpp)

Proposal source SHA-256:
`fa41016b3a5b1efa71a8a807e4ed4c4229e14a92dd73f748cf833a033bd6ba7b`.

| unit | symbol | original RVA | whole size | candidate source/object |
|---|---|---:|---:|---|
| `closure_dxut_matrix_operator` | `??DD3DXMATRIX@@QBE?AU0@ABU0@@Z` | `0x73EF0` (474864) | 46 | `vendor/upstream/saco/d3d9/include/d3dx9math.inl:883-888`; `closure_dxut_direction.obj` |

Candidate object SHA-256 is
`9e6e4af26ab8ffa3471c5ebc69d0b1d04aca7de280a4f558ac4d42ae5b81c6e3`.
The dedicated `.text` COMDAT is 46 bytes, COFF storage class 2, with one
`REL32` (kind 20) at offset 17 to `_D3DXMatrixMultiply@12`. The target is a
reviewed external import-thunk, not an unimplemented function: reference VA
`0x100C5C9E`, import slot VA `0x100E5500`, `d3dx9_25.dll!D3DXMatrixMultiply`,
6-byte thunk, contract SHA-256
`1c3c722c0283d4fb7051df8cc1baa5c15f485a600ee457b8dcff5b857cb90302`.
The pinned SDK source file SHA-256 is
`c79926f99577d6e62b2e264691689fbaa11b987c6303bc644509a0300a00d8c0`.

The proposal defines the SDK inline guard before including `dxstdafx.h` so the
TU sees the SDK declaration and supplies the original inline body once. The
integrator must confirm the resulting COFF section remains a complete 46-byte
COMDAT; this source-level guard is not an exactness claim.

## Integrator checks

Use fresh source/tool hashes and a new proposal ID. Compile each unit alone,
then inspect complete `.text`, EH, COFF relocations, PE fixups, and actual
owners before linking. The GUI root must resolve only to the accepted IME
provider above; the math root must resolve only to the reviewed import thunk.
Run negative controls and full-size comparisons. Do not claim exactness from
these discovery matches alone.
