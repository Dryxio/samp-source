# Vehicle constructor and complete lifetime closure — uncompiled proposal

Baseline checked: death-vehicle-acceptance, 217741 unique bytes. Ped pair1367 is a
separate candidate, no overlap. This proposal adds **2144 principal +36 EH =2180**
unique bytes if all bodies pass; nothing accepted or compiled by this agent.
The earlier informal2244 estimate was arithmetic error. Machine-readable original
ranges/relocations/direct calls and union deduction: vehicle-construction-ranges.json.

## Sources / compile order

1. closure_vehicle_lifetime_helpers_proposed.cpp: GetAt40, GetSlotState29,
   GetTrailer83, Add40, Remove40, RemoveEveryone223, GamePrepareTrain65 (520).
2. closure_vehicle_destructor_proposed.cpp: destructor286 + generated scalar30.
3. closure_vehicle_constructor_proposed.cpp: constructor1308.
4. Seven closure_store_r5*_proposed.cpp script descriptors, one 18-byte owner each;
   three closure_store_*vehicle*_proposed.cpp globals, one actual4-byte BSS each.

Use genuine CVehicle class. Required declaration changes (root only) documented in
vehicle-construction-header-change.txt. No class allocation through a partial view.
Existing genuine layout: CEntity48, trailer48, nativevehicle4C, real objectsizeB8.
The main header change invalidates dependent units, so one staged probe can test
all new units before broad build. This proposal has not received a compile check.

Existing `_rpc_set_car_z_angle` is an internal-linkage SCRIPT_COMMAND in
closure_rpc_spectator_vehicle.cpp, already accepted ED510. Proposed ctor declares
`extern "C" const SCRIPT_COMMAND rpc_set_car_z_angle`; root must expose the sole
existing definition as `extern "C" const SCRIPT_COMMAND ...={...}` so the same
anchor remains `_rpc_set_car_z_angle`. No second descriptor/coverage credit.

All source identifiers/prototypes reviewed against active headers: RequestModel
(int,int), AddDebugMessage member variadic cdecl, GetModelReferenceCount stdcall
WORD(int), pool GetIndex stdcall DWORD/native pointer. No AddInfo/Debug substitution.
Main.h contains preexisting static script descriptors, so emitted used descriptors
must be inspected individually and whitelisted; no whole-data-section assumption.

## Original boundaries and actual lifetime

B83D0/1308 ctor, B88F0/286 dtor, B8B50/30 scalar-deleting dtor,
B8A10/40 Add, B8A40/40 Remove, B7C90/83 GetTrailer,
B7EE0/223 RemoveEveryone, B42B0/65 GamePrepareTrain,1120/40 GetAt,1150/29 Slot.
Real vtable ED728/12 contains B8B50,B8A10,B8A40; every slot implemented here.
Base vtable EC2E4 and base destructor9C280 already real accepted providers.
Ctor EH18 E3FD0: cleanup8→9C280 and handler10→C6526, FuncInfo FBF44.
Ctor xdata FBF3C/36 (unwind8, FuncInfo28).
Dtor EH18 E3FF0, cleanup8→9C280 and handler10→C6526, FuncInfo FBF68.
Dtor xdata FBF60/36. No jump tables. Normal compiler emits scalar dtor.

GetTrailer83 itself calls actual1120 and1150, confirming identity beyond homonym
body matching: WORD bounds2000; pointer1134; BOOL slot3074. FindID real1EB90
provider RPCVehicleMotionPoolView returnsFFFF sentinel, not 0.2.5 bound check.
Read-only pool views are never allocated. Existing MotionPoolView declaration is
copied identically including inline GetAt; new trailer view has separate methods
and real prefixes. Only original1120/1150 bodies count if whole exact; no inline
MotionPoolView duplicate credit.

GamePrepareTrain is 0.2.5 source native scalar-destructor bridge plus R5 initial
vehicle null guard. Native driver460 and pedType598; exclude type0/type1, then
thisECX, vtable slot0, push1 deleting destructor, clear vehicle.driver. It does not
pretend to own a full native ped class or invent a vtable provider.

Dtor texture8F really is a COM reference: B81A0/69 creates it from the text93
through global26EBA0->6D880 and stores result8F; B81F0/31 releases it with the
same vtable+8, pushed-this stdcall as B88F0. IUnknown::Release is the exact ABI.
Neither creation69 nor release31 is required as a called provider for this dtor;
no credit for these unimplemented bodies. Native RemoveModel4089A0 uses cdecl
int, caller removes4 bytes; reproduced source-native bridge with explicit model
local. Calls occur only refcountWORD0, keep87false, loadedmodels>80, model loaded.
Native unknown4B0 is named only by offset; no asserted unproven semantic meaning.

## Original source data — payloads independently read from PE

New script records18 (opcode WORD + char[16]):
ED4E8 r5CreateVehicle A5,"ifffv";
ED4FC r5DestroyVehicle A6,"i";
ED5C4 r5CreateTrain 6D8,"ifffiv";
ED5D8 r5DestroyTrain 7BD,"i";
ED5EC r5VehicleGasTankExplosion 9C4,"ii";
ED614 r5VehicleHydraulics 7FF,"ii";
ED68C r5VehicleTiresVulnerable 53F,"ii".
Used header static descriptors must independently match ED538 disable_marker18
and ED5B0 remove_actor_from_car_and_put_at18 (362,"ifff").
Reused ED510 angle18 (175,"if") belongs to existing provider above.

Actual floats: EC314 0000803e =0.25f; ED468 00003443 =180.0f;
E6118 00000040 =2.0f. None inferred just from relocation coincidence.
ED6FC string "Warning: couldn't create vehicle type: %u" with terminator;
ED6DC "Warning: bad train carriages" with terminator. No padding credit.
Actual BSS:15180C creationflag BOOL,26DFCC lastvehicle pointer,26DFD0 lastidDWORD.
They are unbacked by PE raw bytes, actual zero-init sections; do not claim rawread.
True shared globals pGame26EBAC,pChatWindow26EB80,pNetGame26EB94 are existing.
Imports Sleep E5254 and GetTickCount E5238 are existing actual IAT providers.

## Strict 0.2.5 attribution and differences

0.2.5 directly supplies previously absent vehicle constructor train/carriage
control flow, requesting/waiting/script model creation, actual Add/Remove/trailer,
destruction and occupant evacuation. It is a substantial useful implementation
reference. However no new function is claimed exact before the root's compile.
Ctor changes: R5 seven-argument signature replaces numberplate argument; flag8B
and15180C, null-creation diagnostic, direct creationZ (no025 +0.1), matrixZ+0.25
except subtype2/6, native4B0, many R5 tail fields including COMtexture8F/name93.
Destructor adds trailer detachment, COM lifetime and >80 loaded-model threshold.
GetTrailer adds explicitFFFF sentinel; train preparation adds nullvehicle guard.
Only eventual source-identical separately proven functions should receive direct
025-byte attribution; no blanket2144-byte credit or reused dependency credit.

A complete passing ctor/lifetime would make real vehicle RenderVehicle6C3C0/686
accessible next. That renderer is not proposed or credited in this batch.
