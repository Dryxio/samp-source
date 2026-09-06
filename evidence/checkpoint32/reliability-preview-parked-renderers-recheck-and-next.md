# Bounded recheck after vehicle floating-type correction

No new Object694 or Sprite272 code variant is justified by the available evidence.
Vehicle1308 previously had a true type-width discriminator: double conditional
result fixed entire graph but left FADDqword; mixed explicit casts recovered
FADDdword and all remaining bytes. This specific evidence is absent here.

Object694 already uses original float4 loads, correct45 targets, same stack shape
and call boundaries. Its remaining multiply is merely scheduled between different
centre-zero stores; ESI/EDI uniform exchange starts at first raster call. No extra
float conversion, spill, wrong literal payload or conditional common type isolates
a source correction. Both main radius/centre calls have correct return ABI.

Sprite272 already uses correct FIADD dword local and _ftol input ST0. Two FIADD
instructions are scheduled after bounds-left/top stores rather than before them.
Changing float intermediates to double would be an ungrounded scheduling search:
original never materializes either intermediate, so no store width identifies a
different declared type. Callee native sprite has correct cdecl signature and
out-of-line40-byte implementation. No missing call-clobber proof appears here.
Keep prior failed source/empty-ASM and prefix experiments parked. No added
alignment/no-op/instruction bytes, no changed float operands solely to affect order.

## Independent nearest rendering utility chain

Within textdraw/renderware neighbourhood B20A0/53 -> B20E0/53 -> B2120/25:
GetFirstAtomic(RpClump*) -> RpSkinGeometryGetSkin(RpGeometry*) -> GetClumpSkin.
Unique net131 after229399; no calls to pending Object/Sprite, no allocation/lifetime.
Only existing iGtaVersion26EB28 and true native endpoints. First wrapper chooses
734820 (version1) else734880; second7C7550 else7C7590. Third obtains firstAtomic,
loads real RpAtomic.geometry18, tailcalls geometry skin provider. Native APIs are
cdecl, one pointer argument, EAX pointer return. No new data or EH/tables.

Independent optional GetAnimHierarchyFromClumpB2140/39 calls native734B10,
well-established RpHAnimHierarchy* cdecl(RpClump*). Total170 with that extra body.
Do not conflate the version2 alternative734880 with same-address version1
Get2DEffectAtomic; the actual branch selects GTA executable-version mappings.

Source identity support (primary maintained SDK):
https://github.com/DK22Pac/plugin-sdk/blob/master/plugin_sa/game_sa/common.cpp
(GetFirstAtomic734820 and GetAnimHierarchyFromClump734B10).
https://raw.githubusercontent.com/DK22Pac/plugin-sdk/master/plugin_sa/game_sa/RenderWare.cpp
(RpSkinGeometryGetSkin7C7550). Original two SA-MP wrappers preserve a local
result and native function-pointer stackslot, using normal existing native-call
inline-ASM idiom; pure C++ direct function-pointer call is likely shorter, so this
is a separate native-ABI reconstruction probe rather than pretending source025
body was found. Neither025 nor active base contains these implementations.

Recommendation: prioritize ready owner361/outer549 while retaining Object694
blocker; native skin131/optional39 is fallback incremental useful renderer support,
not a claim that it closes the parked preview bodies. No new code supplied or
compilation requested for that fallback until integrator chooses it.
