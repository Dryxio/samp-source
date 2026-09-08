# SA-MP R5 matching contract

- Objective: reconstruct source that reproduces the frozen R5 binary. Functional
  equivalence alone is insufficient. Do not change original behavior to improve it.
- Reference SHA-256: b72b5dbe725f81864ca3f78bc7063bda56cc05fc7188af822fa7a754432553a2.
- Keep proprietary binaries, disassembly captures and build products in ignored
  `private/` or `build/`. Never commit a reference executable.
- Use normal C++ first. Symbolic assembly exceptions require explicit labeling.
  Never count `_emit`, incbin or copied instruction bytes as reconstructed code.
- Record complete function ranges and provenance. Never truncate comparison to
  the shorter body or mask arbitrary addresses, calls, branches or immediates.
- Every COFF relocation must resolve through a reviewed symbol manifest. Unknown
  symbols/types and mismatching targets fail closed. Original PE relocations must
  agree. Separate raw exact, resolved-function exact and whole-file exact claims.
- Unimplemented external functions remain dependencies, not accepted coverage.
- Preserve failed attempts as evidence. Acceptance needs fresh source/tool hashes,
  full-byte comparison, negative controls and relevant boundary/ABI checks.
- Do not alter legacy repositories or deploy candidates to GTA during static
  matching work. A test capsule is not a complete replacement DLL.

## Checkpoint 3.2 integration handoff

- For supported Ob1 batches, use a pinned proposal with
  `tools/integration_pipeline.py`; see `docs/integration-pipeline.md`.
  Coverage, actual symbol owners and emitted dependency closure must be checked
  before linking. Researchers hand off isolated sources and complete reviews;
  only the integrator runs the shared VM or changes the active source tree.
- Do not bypass a rejected proposal by reverting to accumulated global exports.
  Resolve the reported issue and preserve the attempt under its original ID.
- V1 does not support header/profile/Ob2 owner changes. When a batch needs those,
  record the limitation and apply the same prelink controls explicitly through
  the existing workflow; all final acceptance requirements above still apply.
