# R5 integration pipeline (v1)

`tools/integration_pipeline.py` assembles the existing compiler, closure review,
linker qualifications and final acceptance gate from one pinned proposal.
It does not replace the byte/relocation/target checks or certify a source manifest
merely because its status says PASS.

## Researcher handoff

Write JSON in an isolated folder under `build/`. Version 1 accepts canonical
Ob1 `.cpp` units in `client/saco`; changes to headers, compiler profiles or Ob2
owners require separate integration. A proposal contains:

- `version: 1`, unique lowercase `id`.
- `base`: `prefix`, and pinned `proof`, `contract`, `seeds`.
- `roots`: objects with `unit`, mangled `symbol`, integer `rva` and whole `size`.
- `reviews`: pinned complete source-region manifests, including target bindings.
- Optional `sources`: pinned staged files plus `target` and `before_sha256`
  (null only when the target does not exist).
- For `run`, `review_note`, pinned `qualification_recipe` and
  `qualification_inputs` containing every entry of `LEGACY_INPUTS` in the tool.

A pinned input is `{"path": "repo/relative/path", "sha256": "..."}`.
The v1 recipe is `build/qualify_trial30b_link.py`. Its transitive scripts and
reviewed artifacts must still exist locally. Deleting all of `build/` therefore
prevents running this version; it is not yet a self-contained clean build.

Researchers still investigate source and provide complete reviewed source
manifests. This first version automates integration of those candidates, not
semantic review or arbitrary compiler-variant search.

## Commands

Run from the repository with its Python environment:

```sh
.venv/bin/python tools/integration_pipeline.py plan build/proposal.json
.venv/bin/python tools/integration_pipeline.py audit build/proposal.json --objects build/cp32-candidate
.venv/bin/python tools/integration_pipeline.py run build/proposal.json
```

`plan` checks pinned baseline artifacts, current coverage, whole root ranges,
owners and source preconditions. Potential root bytes exclude accepted bytes;
they do not predict the final gain, which may include other fully matched code.

`audit` merges actual candidate objects into a private baseline snapshot and
traverses the emitted closure. It rejects new divergences, missing reviews,
changed accepted regions, unreviewed externals and competing definitions. Newly
selected roots are checked even when both competing COMDATs already existed.
An unchanged historical owner conflict is not treated as a newly introduced one;
actual linked selection remains subject to the final gate.

`run` holds a nonblocking integration lock, checks host commands and guest compiler processes, freezes inputs,
backs up and installs proposed sources, then runs the existing incremental probe through a short PowerShell wrapper.
The generated unit list is passed as a file to avoid a very long guest command
line.
After the closure audit it generates a private exports file from the certified
baseline and roots, links once, qualifies the actual linked providers, builds the
reviewed contract and calls the unchanged acceptance gate. Compilation caching
remains the existing probe's responsibility. Do not run other VM operations or
edit frozen inputs while this command runs. The guest preflight times out after
30 seconds; other stages have a 15-minute ceiling. After a timeout, inspect guest
processes before retrying: stopping the host command does not prove guest work
stopped.

The linker now accepts `-ExportsFile`; omission preserves its previous behavior.
The pipeline always supplies its frozen file and checks the copied hash.

A failed run preserves sources, backups, objects, logs and diagnostics. It does
not silently roll back or accept partial work. Inspect the failure and use a new
proposal ID. No automatic commits, pushes or deployments occur. After acceptance,
the integrator reviews the generated proof, updates continuation/documentation,
and commits the coherent lot. The acceptance gate remains the authority for
coverage and includes full source/linked comparisons and existing negative/ABI
controls.

`--historical` permits read-only planning/auditing against an earlier baseline;
`run --historical` is forbidden. Historical replays never earn coverage.

## Regression checks

```sh
.venv/bin/python -m unittest discover -s tests/integration -v
```

These tests intentionally use retained historical objects. They require the
trial-F build and manifests; they must fail visibly if that evidence is missing.
They cover a positive lot, already-covered roots, a stale baseline, Router's
unclosed emitted dependencies, the actual 77/84-byte COMDAT conflict, a duplicate
strong owner, changed hashes, tampered review bindings, source preconditions and concurrent
integration lock acquisition.

See `evidence/checkpoint32/integration-pipeline-validation.json` for measured
validation and remaining limits. No throughput or token saving is inferred from
these correctness tests; that needs the next comparable timed trial.
