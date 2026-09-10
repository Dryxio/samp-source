<div align="center">

# SAMP Source

**Rebuilding a multiplayer classic. One exact byte at a time.**

SA-MP 0.3.7 R5 · C++ · AI-assisted reverse engineering

</div>

The goal is simple: rebuild **`samp.dll` from source and produce a file identical to the original**.

SAMP Source is a software preservation project for San Andreas Multiplayer. We reconstruct the code, compile it with the original-era tools, and check the result against R5. Every accepted match comes with evidence you can inspect.

## How it works

**Reconstruct → Compile → Compare → Keep the exact matches.**

Functions are checked in full, including their references, call targets and jump tables. Differences are never hidden to make a result pass, and original machine-code blobs are never pasted into the reconstruction. Documented assembly is used where needed.

## Progress

**473,494 code bytes verified — 50.87% of the original code section.**

Already covered: parts of actor creation and deletion, entity geometry, weapon lookup, game commands, native pools, player-pointer records, raster helpers, camera/audio wrappers, player state, weapon skills, native task wrappers, menus, RenderWare and shared utilities.

| Milestone | Status |
| --- | --- |
| Toolchain, binary mapping and initial dependencies | Established |
| Reconstruct and match the client | In progress |
| Integrate a complete client | Ahead |
| Reproduce the entire DLL, byte for byte | The finish line |

This is an early reconstruction, not a playable replacement DLL. Matching individual functions is progress toward full-file identity, not proof of it.

[Verified coverage](evidence/checkpoint32/coverage-current.json) · [Latest accepted batch](evidence/checkpoint32/overnight-hash-archive-four-acceptance.json) · [Development log](evidence/checkpoint32/PROGRESS.md)

## Explore or contribute

- **[Source](client/saco/)** — the client reconstruction and imported reference code. Presence here does not imply a verified match.
- **[Tools](tools/)** — compilation, binary comparison and validation.
- **[Evidence](evidence/)** — accepted results and their limits.
- **[Matching rules](AGENTS.md)** — what qualifies as an exact match.

Reproduction requires your own R5 DLL, Visual Studio .NET 2003 and Python. The current build automation uses a Windows VM through Parallels on macOS. The original DLL, build artifacts and compiler are not included.

## Credits

Built on earlier community work, including [dashr9230/SA-MP](https://github.com/dashr9230/SA-MP), with [source provenance recorded](config/checkpoint32/provenance.json). Prior work provides leads; binary comparisons establish the matches.
