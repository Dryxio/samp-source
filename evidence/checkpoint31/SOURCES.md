# Pinned dependency sources

`vendor/upstream/` is a byte-preserving local snapshot of selected inputs from
dashr9230/SA-MP, commit `cd3ec8b7ba297ca02402c7f22699dfd5e72e20bd`.
Every file is listed with a SHA-256 in `config/checkpoint31/vendor.json`.
Existing copyright and license notices are preserved. This project does not
relicense RakNet, Microsoft's SDK samples/headers, or other imported material.

The snapshot includes the RakNet fork, DXUT, the required DirectX headers,
utility implementations and game declarations needed to compile DXUT. Only
the 50 source units in `config/checkpoint31/build.json` are compiled. Additional
headers and source files provide provenance and candidates, not accepted coverage.
Game headers remain upstream hypotheses unless separately supported by matching
regions and ABI checks.

Upstream `MATCH`/`TODO` comments are preserved as historical comments. They have
no authority over this repository's verifier. Compiled source availability,
successful integration and exact R5 matching are reported separately.

Import libraries and runtime DLLs remain in ignored `private/checkpoint31/`.
Their required hashes are recorded in `config/checkpoint31/private-inputs.json`.
The original SA-MP binary and IDB are not redistributed here.
