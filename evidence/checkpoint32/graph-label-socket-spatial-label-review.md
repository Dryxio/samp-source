# One independent 3D-label family: 564 source bytes potentially new

Complete accepted union was loaded with accepted_code(logger-queries-acceptance.json):262111, including recursively composed bases. All nine proposed code ranges are wholly uncovered. Plate230/request172 pending elsewhere do not overlap. Scoreboard ctor/recenter209 already covered; ResetDialogControls342 still requires real outer ListBox constructor/vtable closure, so no fake allocated GUI view is proposed.

| Identity | RVA | bytes |
|---|---:|---:|
| Complete label owner constructor | 6BBB0 |25|
| Destructor |6BBD0|28|
| OnLostDevice |6BBF0|14|
| OnResetDevice |6BC00|14|
| IsLineOfSightClear |6BC10|96|
| Begin |6BC70|16|
| End |6BC80|14|
| Draw |6BC90|351|
| actual GameGetInternalAim |9C980|6|

Owner has exactly two pointers: device0, sprite4. Actual startup allocates8 at C4C10/C4C12, constructor called C4C33, result stored into singleton26EB5C atC4C45. Existing base CLabel constructor is the same two-field owner. Proposed descriptive complete class includes size8 and offset4 assertions, with no invented fields, vtable or padding. No pointer-view allocation. Destructor really invokes scalar operator delete C648A on sprite and nulls it (R5 behavior, also used by accepted player tags), not COM Release. Lost/reset COM slots30/34; sprite Begin20 with flag16; End2C.

Draw identity additionally anchored by the original object-label caller6DA60 (calls Begin6BC70/Draw6BC90/End6BC80), whose shared renderer resolves from the real singleton. Draw takes position pointer, text pointer, DWORDcolor, BOOLshadow (32-bit original stack read), booltestLOS (byte original stack read). It calls IsLineOfSightClear only when the fifth flag is true, then GetViewport slotC0, projects with actual view140D70/projection140DB0 and local identity world matrix, rejects z>1.0f and renders RECT{x,y,x+1,y+1} with flags101h. Real CFontRender6B4E0 and D3DXVec3ProjectC5C80/E5514 are already accepted/imported; module d3dx9_25.dll, library d3dx9.lib. Matrix globals retain their actual accepted owners. D3DXMatrixIdentity already accepted; ordinary inline definition should produce the observed16 immediate stores under Ob2. No import thunk source credit.

Visibility96 uses actual GameGetInternalAim9C980, named by original decorated inventory and existing game/aimstuff.cpp source. It returns pointer stored at1039C8; full original pointer data is B6F32C. CAMERA_AIM complete existing type has pos1x/pos1y/pos1z atC/10/14, exactly the three original loads; these are the camera ray endpoint. ScriptCommandB2310 accepted. Original descriptor EA618/18 is opcode06BD and full parameter bytes ffffffiiiii+fiveNULs. Crucially0.2.5 game/scripting.h:119 supplies the missing exact named get_line_of_sight descriptor and comments: startXYZ/endXYZ/solid/vehicle/actor/obj/fx. Flags1,0,0,1,0 preserve original queries against solids and objects. A null aim or false script result returns FALSE, otherwise canonical TRUE.

Sources: closure_spatial_label_renderer_proposed.cpp (eight methods), closure_label_internal_aim_proposed.cpp (true named6-byte provider), closure_store_label_internal_aim_proposed.cpp (actual pointer4), closure_store_label_line_of_sight_proposed.cpp (whole descriptor18). Root must transfer the sole existing pcaInternalAim definition from game/aimstuff.cpp if that TU is active, and avoid a second GameGetInternalAim definition. No new pointer is initialized to a fabricated address; data comes from existing base native binding and original payload. Minimal store TUs prevent unrelated header sentinels in complete regions. No singleton26EB5C store needed to accept these bodies.

No EH observed in this family. Literal1.0f atE5B2C/4 already has actual ownership; verify payload3F800000, not code-only reference. Original native pointerB6F32C is fixed GTA address, not PE-relative reloc target. New dataset spatial-label-original-manifest.json has whole code/data hashes and all original relocation sites. Net candidate code564, new data at most22 (pointer4+descriptor18); gate handles actual unique union. Existing float and matrices gain0.

Source025 gain: exact missing descriptor18 and its semantic parameter documentation are useful; normal source code is reconstructed R5/base, strict direct025 codecredit0. First bounded probe Ob2 recommended only to reproduce observed MatrixIdentity/helper inlining. No symbolic assembly, copied bytes or masked fields.

## Root owner reconciliation correction

The active repository already owns pcaInternalAim in client/saco/closure_store_internal_aim.cpp at the real1039C8 address with B6F32C payload. Reuse that owner unchanged. Do not compile closure_store_label_internal_aim_proposed.cpp, do not transfer/edit aimstuff.cpp, and do not count another4 data bytes. The historical proposal file is retained solely as superseded evidence. The probe comprises spatial_label_renderer, label_internal_aim, and store_label_line_of_sight only. Potential new data is at most descriptor18; code564 remains unchanged. The accepted-union utility alone did not expose this owner attribution, so final union/provider reconciliation remains authoritative.

## Actual trial1 complete preflight

cp32-spatial-label-trial1 (Ob2): all nine principal bodies match exactly564 on the first probe. spatial-label-reviewed-manifest.json records11 full sections,15 bindings, full resolved bytes, complete PE relocation site equality both ways, original and object hashes. preflight_spatial_label.py reproduces checks. Full descriptor18 matches original; existing float1.0f4 also rechecked. Existing pcaInternalAim1039C8 payloadB6F32C checked separately with its existing owner unchanged. EH0.

D3DXMatrixIdentity59 is inlined into Draw as the same complete16 stores. The emitted unused D3DXMatrixIdentity59 and no-op constructors3/3 receive zero additional source credit. No unknown library provider or incomplete allocation entered this closure. D3DXCreateSprite C5C74 and D3DXVec3Project C5C80 remain real imported providers already established by prior renderer/health; do not count their thunks. Whole code564 plus at most descriptor18 is candidate gain582, subject to root actual linked union and acceptance.
