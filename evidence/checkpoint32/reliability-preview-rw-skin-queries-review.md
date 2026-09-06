# Native RW query chain170 — ready integrator probe, uncompiled

closure_rw_skin_queries_proposed.cpp contains four actual typed wrappers:
GetFirstAtomic B20A0/53, RpSkinGeometryGetSkin B20E0/53, GetClumpSkin B2120/25,
GetAnimHierarchyFromClump B2140/39. Independent unique170 after229399.

First three form a real chain: B2125 calls B20A0, reads returned RpAtomic.geometry
at18, tailcalls B20E0. GetClumpSkin intentionally has no extra null check.
Opaque external native RpGeometry/RpSkin/RpHAnimHierarchy types are pointer-only;
RpAtomic from existing header not expanded/allocated, geometry offset view only.

Exact native ABI: all cdecl, one pointer argument, EAX pointer result. Two version
choices inspect real iGtaVersion26EB28: value1 => US1.0 734820/7C7550;
other=>734880/7C7590. Hierarchy wrapper uses734B10 without a version split, as R5.
No inferred alternate native address, newly invented callback, fake vtable or code
bytes. Local result +nativeFunction and EAX bridge preserve actual original
32-bit wrapper calling pattern from existing game/rwstuff.cpp; only the native
call itself uses named inlineasm. All locals retain real purpose and no padding.

Sources for primary API identity:
https://github.com/DK22Pac/plugin-sdk/blob/master/plugin_sa/game_sa/common.cpp
https://raw.githubusercontent.com/DK22Pac/plugin-sdk/master/plugin_sa/game_sa/RenderWare.cpp
SDK identifies GetFirstAtomic, GetAnimHierarchyFromClump and geometry skin APIs;
R5 instructions independently prove version alternates and geometry18 access.

No new data, literals, EH or tables; only iGtaVersion existing store. Direct025
credit0 (no body found). Full ranges/hash/relocations in rw-skin-queries-manifest.json.
