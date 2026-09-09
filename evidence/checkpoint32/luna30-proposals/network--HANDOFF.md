# RakNet nested vendor batch — UNCOMPILED

## Replacement proposal — use this one only

The earlier 637-byte `RakPeer` constructor proposal is preserved below as
rejected evidence. Do **not** install it: its constructor writes the full
`RakPeer` VFT and therefore has many unimplemented virtual targets. This new
proposal contains no `RakPeer` constructor and makes no VFT claim.

Stage [rakpeer_nested_candidate.cpp](rakpeer_nested_candidate.cpp) as
`client/saco/rakpeer_nested_candidate.cpp`. It is C++03-compatible: there is no
out-of-class `= default`. The externally visible `__declspec(noinline)` helper
uses ordinary placement new only to force materialization of the true implicit
`RemoteSystemStruct` constructor; the helper is not a root and must not be
credited as a constructor body.

Replacement roots, all complete and nonvirtual:

| symbol | reference RVA | size |
|---|---:|---:|
| `??0RemoteSystemStruct@RakPeer@@QAE@XZ` | `230208` | `90` |
| `??0?$List@PAUBanStruct@RakPeer@@@DataStructures@@QAE@XZ` | `233344` | `13` |
| `??0?$List@PAVPluginInterface@@@DataStructures@@QAE@XZ` | `233600` | `13` |
| `??0?$List@UPlayerIDAndIndex@@@DataStructures@@QAE@XZ` | `236112` | `13` |
| `??0?$OrderedList@UPlayerID@@UPlayerIDAndIndex@@$1?PlayerIDAndIndexComp@@YAHABU1@ABU2@@Z@DataStructures@@QAE@XZ` | `241968` | `12` |

Total proposed code is 141 bytes, wholly uncovered against the unchanged
428758-byte baseline. The pinned inputs are
[rakpeer_nested_seeds.json](rakpeer_nested_seeds.json),
[rakpeer_nested_discovery.json](rakpeer_nested_discovery.json),
[rakpeer-nested-rank.json](rakpeer-nested-rank.json), and
[rakpeer-nested-baseline-owners.json](rakpeer-nested-baseline-owners.json).
The owner index confirms the RemoteSystemStruct dependencies are the accepted
ReliabilityLayer, BitStream, and RPCMap providers; the template constructors
use only the ordinary current vendor headers, with OrderedList depending on
the included `List<PlayerIDAndIndex>` constructor.

Root compile must first verify that the placement-new helper emits the actual
`??0RemoteSystemStruct@RakPeer@@QAE@XZ` symbol and that all five roots have the
expected complete ranges. If it does not, stop and report blocked; do not add
VFTs or broaden this batch.

Status: `UNCOMPILED`. No matching or acceptance claim is made. The integrator
must stage `rakpeer_ctor_candidate.cpp` as
`client/saco/rakpeer_ctor_candidate.cpp`, compile it through the normal
sequential root probe, and review the resulting object’s complete code/data/EH,
COFF relocations, PE targets, and actual selected owners.

## Selected roots

All ranges are complete reference functions and are distinct. The six roots
sum to 637 uncovered code bytes; `tools/rank_uncovered_candidates.py` with
`accepted_code` against `evidence/checkpoint32/pipeline-next-two-acceptance.json`
reported the same 637 bytes from a 428,758-byte accepted union.

| unit | symbol | reference RVA | size |
|---|---|---:|---:|
| `rakpeer_ctor_candidate` | `??0RakPeer@@QAE@XZ` | `253520` | `496` |
| `rakpeer_ctor_candidate` | `??0RemoteSystemStruct@RakPeer@@QAE@XZ` | `230208` | `90` |
| `rakpeer_ctor_candidate` | `??0?$List@PAUBanStruct@RakPeer@@@DataStructures@@QAE@XZ` | `233344` | `13` |
| `rakpeer_ctor_candidate` | `??0?$List@PAVPluginInterface@@@DataStructures@@QAE@XZ` | `233600` | `13` |
| `rakpeer_ctor_candidate` | `??0?$List@UPlayerIDAndIndex@@@DataStructures@@QAE@XZ` | `236112` | `13` |
| `rakpeer_ctor_candidate` | `??0?$OrderedList@UPlayerID@@UPlayerIDAndIndex@@$1?PlayerIDAndIndexComp@@YAHABU1@ABU2@@Z@DataStructures@@QAE@XZ` | `241968` | `12` |

The explicit seed/ranking inputs are [rakpeer_init_seeds.json](rakpeer_init_seeds.json),
[rakpeer_discovery.json](rakpeer_discovery.json), and [rakpeer-rank.json](rakpeer-rank.json).
The true-owner diagnostic is [rakpeer-baseline-owners.json](rakpeer-baseline-owners.json),
generated with `tools/index_symbol_owners.py` against
`build/cp32-pipeline-next-two-linked1` and the pinned contract.

## Source and layout

[rakpeer_ctor_candidate.cpp](rakpeer_ctor_candidate.cpp) copies the complete
normal-C++ `RakPeer::RakPeer` body from
`vendor/upstream/raknet/RakPeer.cpp` (source lines 164–208; vendor file SHA-256
`71b46e7918c933c5eb7a171d5c76d43833568e3193a0244b79e40bec4854c61b`). It uses
the unchanged `../raknet/RakPeer.h` class layout. `protected` is exposed only
around the header to explicitly instantiate the real member-template types;
there is no layout, profile, VFT, or header edit. The
`RemoteSystemStruct` constructor is explicitly defaulted solely to request the
compiler-generated layout constructor seen in the original RakPeer TU.

Explicitly instantiated missing constructor providers are the two pointer
`List` types, `List<PlayerIDAndIndex>`, and the real `OrderedList` type. The
already-accepted requested/buffered/packet queue providers are not duplicated.

## Dependency owners

These are the owners reported by the baseline owner index, not inferred global
exports:

| dependency | actual baseline owner | accepted reference region |
|---|---|---|
| `??0ReliabilityLayer@@QAE@XZ` | `closure_vendor_reliability_lifecycle` | RVA `298512`, size `268` |
| `??0BitStream@RakNet@@QAE@XZ` | `closure_cmd_bitstream` | RVA `128816`, size `33` |
| `??0RPCMap@@QAE@XZ` | `eval_vendor_rpc_map` | RVA `333360`, size `18` |
| `??0SimpleMutex@@QAE@XZ` | `eval_vendor_simple_mutex` | RVA `341984`, size `14` |
| `??0SingleProducerConsumer@RequestedConnectionStruct...` | `eval_vendor_rakpeer_queue_lifetime` | RVA `233856`, size `127` |
| `??0SingleProducerConsumer@BufferedCommandStruct...` | `eval_vendor_rakpeer_queue_lifetime` | RVA `234384`, size `102` |
| `??0SingleProducerConsumer@Packet*...` | `closure_vendor_packet_spc_lifecycle` | RVA `234832`, size `102` |
| `??0Queue@Packet*...` | `closure_vendor_network_template_members` | RVA `235280`, size `34` |
| `?reset@RSACrypt<u256>...` | `eval_vendor_rakpeer_rsa` and identical historical owner `eval_vendor_rakpeer_send_handshake` | RVA `241872`, size `89`; do not double-credit |
| `?AddReference@StringCompressor@@SAXXZ` | `closure_vendor_string_compressor` | RVA `345744`, size `112` |
| `?AddReference@StringTable@@SAXXZ` | `closure_vendor_string_table` | RVA `347488`, size `51` |

The RakPeer constructor has a VFT fixup to `??_7RakPeer@@6B@`. The full VFT is
not accepted; its actual reference/data owner is retained in
`build/agent-independent/plugin-owner-discovery/raknet_RakPeer.obj`, SHA-256
`989ade9ed4e8ecbf9f061680a39b57dd03448bf381a7e3f35cad6ee90b3dc505`, at table
RVA `953840`. Review this data target explicitly; no VFT bytes are claimed in
the 637 code-byte total.

## Excluded hypothesis

[rakpeer_init_candidate.cpp](rakpeer_init_candidate.cpp) is retained as
UNCOMPILED evidence only. Its complete `RakPeer::Initialize` body is a grounded
659-byte candidate at RVA `272288`, but its thread entry reaches the
unaccepted `RunUpdateCycle`; it is therefore not part of this closed batch.
Do not retry RouterSend or Cell174 without new evidence.

## Hashes

- `rakpeer_ctor_candidate.cpp`: `9002c290a3d2267d1ec84098427c155a34fd53c4aab03138d1056bafd9ca1749`
- `rakpeer_init_candidate.cpp`: `b199cc18923ff93fac48d9594d100e85d29893b29c7868f7c3787d359e7e55ab`
- `rakpeer_init_seeds.json`: `895cd93185a53f7f617395610c00ed277756a845b847e7673fcbe8110516a486`
- `rakpeer-rank.json`: `6b1bb47bfb25d737f871da3afa9c9471c5e510f9afeeea42a133d50ba13c9bf2`
- `rakpeer-baseline-owners.json`: `28300e910ad383a5d1a7549afb870f94729df733529d0134609804fd1995b60c`
