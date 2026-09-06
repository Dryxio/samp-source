# Independent complete preflight — candidate2887, not acceptance

Runs specified by integrator verified read-only. Complete code+EH union adds2887
against huffman-ped accepted223896. Lifetime2180 +renderer707; duplicate emitted
scalar-deleting30 counted once. No separate data, helperinline, base-constructor,
base-destructor, unused code, or reused provider credit.

90 inferred bindings independently agree with accepted providers and reviewed new
identities. 185 COFF fixups resolved across18 complete sections; exact original PE
relocation sets checked in both directions. All18 sections match whole original
bytes, including principal bodies, actual scalar30 from both constructor/dtor
objects, three complete EH sections and three complete36-byte xdata sections.

18 exact source payloads checked: seven whole isolated SCRIPT_COMMAND18 sections,
seven float4 records, two real warning strings with terminators, and two header
static18 descriptors (disable_markerED538 / remove_actorED5B0). Floats include
constructor corrected actual0.25f EC314=0000803e; no double8 substitution remains.
Four actual BSS4 owners verified whole uninitialized COFF sections and real PE
zero-fill tails (creation15180C,lastvehicle26DFCC,lastid26DFD0,preview26EBE0).
No RakNet sentinel copied into any new owned descriptor or BSS region.

Two complete compiler-emitted CVehicle vtable12 sections resolve exactly to
ED728: deleting destructorB8B50, AddB8A10, RemoveB8A40. MSVC weak external
??_ECVehicle has actual COFF storage105 auxiliary tag pointing to real emitted
??_GCVehicle strong body (characteristic2). Both emitted G bodies independently
resolve to originalB8B50/30 and call true dtorB88F0/operator deleteC648A. This is
compiler provenance, not a hand-invented destructor alias or false vtable.
Root final linked MAP must preserve these reviewed strong/weak identities.

Actual new call chain is closed within batch plus previously accepted providers.
Native bridge in GamePrepareTrain65 follows source025 and preserves driver type
checks, native scalar virtual destructor ABI. Destructor COMtexture field8F uses
real IUnknown slot2/8, stdcall pushed this; source8F producer/releaser independently
identified in initial audit. Native streaming4089A0 cdecl bridge preserves model
parameter and4-byte stackcleanup. Original behavior including uninitialized-on-
failure wrapperfields, train-specific branches and resource failures is retained.

## Strict direct0.2.5 attribution

Function | matched candidate bytes | direct025 credit | rationale
Add B8A10 |40|40|body directly recovered from vehicle.cpp Add
Remove B8A40 |40|40|body directly recovered from vehicle.cpp Remove
RemoveEveryone B7EE0 |223|223|same full driver+7passenger evacuation implementation
GetSlotState1150 |29|29|same inline body, MAX_VEHICLES2000 also in025; R5 field translation only
GetAt1120 |40|0|equivalent implementation already available in active MotionPoolView; no new025 gain
GetTrailerB7C90 |83|0|hybrid R5FFFF test and structure; adapted reference useful
GamePrepareTrainB42B0 |65|0|hybrid R5 initialvehicle-null guard plus025 native bridge
CtorB83D0 |1308|0|hybrid R5 signature, fields, creation/matrix/lifetime; source025 essential foundation
DtorB88F0 |286|0|hybrid new trailer/COM/model eviction conditions
ScalarB8B50 |30|0|compiler generated, no manual source transfer
RenderVehicle6C3C0 |686|0|R5 reconstruction; absent025 preview system
EH57 |57|0|compiler generated

Conservative direct transfer subtotal **332**; coverage candidate subtotal **2887**.
The larger practical gain is real025 lifetime/constructor source made the R5
adaptation tractable; do not call the entire batch a direct025 transfer.

Reproducible read-only verifier: preflight_vehicle_pair.py.
Compact machine-readable evidence: vehicle-lifetime-preview-full-preflight.json.
Final acceptance remains integrator's fresh build/link/strict gate.
