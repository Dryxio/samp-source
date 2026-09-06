# Grounded revisit: entity renderer138; matrix91/82 and preview694 remain parked

9FB20/138 has no found0.2.5 body, but precise native identities are now established. Native entity vtable offset44/index17 is **PreRender**, not Render(index18/48). 733160 is CVisibilityPlugins::SetupVehicleVariables(RpClump*), cdecl; 553260 is CRenderer::RenderOneNonRoad(CEntity*), cdecl. This corrects previous loose labeling of these operations. Sources:
https://raw.githubusercontent.com/DK22Pac/plugin-sdk/master/plugin_sa/game_sa/CEntity.cpp
https://github.com/DK22Pac/plugin-sdk/blob/master/plugin_sa/game_sa/CVisibilityPlugins.cpp
https://github.com/DK22Pac/plugin-sdk/blob/master/plugin_sa/game_sa/CRenderer.cpp

Proposed closure_preview_entity_render_proposed.cpp reconstructs whole control flow normally and uses established source-native inlineasm calls. Pointer-only receiver entity40; native vtable0, RwObject18, WORDmodel22. No allocated native class/fullvtable, no fake providers. Initial vtable dereference occurs before helper null guard as original; no invented early safety return. Local inline GetRwObject matches existing accepted9FA60 semantics but grants no separate helper credit. Native PreRender called unconditionally; actual nonroad-render called only if original savedRwObject nonnull, with separate vehicle-range and nonvehicle source branches retained as in original. Range400..611 evaluated before and after PreRender, so preserve potential native model change. Existing scalar literals only; no PE relocation, EH or table. First probe not yet performed;138 net potential.

## SetMatrixAndUpdate91

Compared historical closure_world.obj89 to original91. Difference is only initial native pointer/vtable materialization: original MOV ECX,EAX; MOV EDX,[ECX]; store vtableEDX; store entityEAX versus compiled storeentityEAX; MOV EAX,[EAX]; storevtableEAX. Everything after native Remove(slot3/C), 64-byte matrix copy, SetMatrix9EBC0, update9EC80 and native Add(slot2/8) has same semantic sequence. Native field bridge precedent gives a possible explicitly symbolic solution, but no new normal C++ source hypothesis warrants another old89/91 compile. No proposed source here.

Still-blocking Update9EC80 old80 vs82 removes redundant second entity-null test and colors pointer locals differently. Original explicitly rechecks EAX before storing local rwObject/matrix/entity and then checks both others. Compiler eliminates redundant test from existingbase C++; exposing volatile fields would change loads and semantics. No0.2.5 newbody solves this. Thus91 alone would remain unclosed until real82 updateprovider matches; neither counted.

## Object preview694

Corrected argument order is now known. Remaining ESI/EDI swap has no established source/data cause: aggregates and explicit memset were tried and the latter changed size to684, without resolving color swap. All45 actual providers, fourfloat payloads and receiver offsets agree after signature correction. No evidence supports a different data layout or dependency identity that would explain registers. Keep parked rather than invent a class member, alias or padding to steer compiler output. No alignment no-op emitted.
