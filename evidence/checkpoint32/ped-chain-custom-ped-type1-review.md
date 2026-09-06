# Type1 custom ped closure — uncompiled proposal

Eight complete bodies total **823 new principal bytes** against screenshot-model acceptance 210955. Zero accepted credit at proposal time. Manifest includes full original hashes, ranges, PE relocations and actual direct calls; original capture contains all eight bodies. No attached tables or EH are apparent in these bodies; compiler-generated regions must still be checked by root.

## Integration

Compile four isolated proposed TUs: closure_custom_ped_model_proposed.cpp; closure_ped_clump_stream_proposed.cpp; closure_ped_model_clump_setter_proposed.cpp; closure_custom_ped_load_manager_proposed.cpp. Standard current /Zp1 packing is required: manager paths9/10E; entry baseC, model10, checksums46/4A, ready58, retry59. These are pointer-only views, never new allocated full manager classes.

The chain is D2C0/94 → C650/281 → A7BD0/81 → A7A80/76, A7B60 accepted, B1F10/137 → B1E80/65 + B1ED0/53, and B45D0/36. C650 calls the PED installer A7BD0, whereas accepted C770 calls OBJECT installer A7C30. Their shared file-resolution algorithm does not justify aliasing their identities.

## Reused owners and data

GetModelInfo A7A40, IsActorPedModelValid B3DD0, bool LoadCustomModelTexture A7B60, OpenCustomModelStream B1DC0, CloseCustomModelStream B1E00, UnFuck AA4C0, calloc C6982/187, sprintf C7353 and IsFileOrDirectoryExists B5DC0 are already implemented. Reuse their actual symbol mappings. All new direct calls are either these providers or bodies in this proposed eight-function chain.

Reuse extendedModelInfos pointer114B08/4, loading flag15181C/1 and iGtaVersion26EB28/4. No new dummy global. The three format arrays are extern owners already accepted: E5CC8/3 `%X`, E5CA8/12 DFF and E5CB4/12 TXD. Existing source bytes must still be checked at final link; each path has one literal backslash. Source uses no new float constants. calloc requests68 items of1 byte and memcpy copies68; preserve original allocation failure/leak semantics rather than adding cleanup or null checks.

## Native ABI identities

B1E80 selects RwStreamFindChunk native7ED2D0 for iGtaVersion1 or7ED310 otherwise, cdecl four arguments (stream, chunkType, length, version). B1ED0 selects RpClumpStreamRead74B420 or74B470, cdecl one argument. Chunk type16 is RenderWare clump. B1F10 opens stream(2,1,path), uses the same already accepted native loading patch protocol at41B1D0 as object-clump loader: permission helper, writeC3, loading1, read, close, restore64/loading0 on both paths. No GTA execution is requested.

B45D0 stores the clump pointer at native model-info1C then calls the real virtual CClumpModelInfo::SetClump slot16 (byte40), ECX=this and one callee-popped argument; wrapper itself stdcall ret8. Primary SDK corroboration: https://raw.githubusercontent.com/DK22Pac/plugin-sdk/master/plugin_sa/game_sa/CClumpModelInfo.cpp and CPedModelInfo.h under the same directory (native ped size44). No fake complete vtable class. The small native inlineasm bridges match the established source ABI workaround, explicitly not a portable C++ claim.

## Provenance and expected first-probe risks

Direct new 0.2.5 credit **0**: no matching custom ped model implementations found in its client; this is R5 reconstruction reusing our already accepted object loader, native/RW wrappers and literals. Existing source patterns accelerate this closure, but reused bytes are excluded.

The C650 resolver is explicitly labeled symbolic exception, adapted from already accepted C770 after its three bounded C++ failures. All other control flow is normal C++ except the narrow native ABI bridges. Potential normal-C++ first-probe issue: original A7A80 preserves sourceModel in EDX across GetModelInfo, whose actual body does not clobber EDX; separately compiled extern calls may cause a save/reload instead. Do not bind a different provider or assume match to hide this. Probe first, then diagnose only actual differences.
