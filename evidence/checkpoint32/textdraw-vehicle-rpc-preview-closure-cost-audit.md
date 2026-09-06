# Preview closure: considerably larger than513+36

Diagnostic only; no new source proposal. Current contract textdraw-init used for provider status. Complete inventory chunk sizes and direct call identities are captured in preview-direct-provider-graph.json. Sprite is excluded and no variants restarted.

| Priority | Missing direct provider | Size | Source / closure assessment |
|---|---|---:|---|
|1|B3DB0 model-info-present predicate|19|Already accepted GetModelInfoA7A40 plus BOOL normalization; no new function provider. Base/025 GetModelInfo source is available but this predicate has no distinct found025 body. Cheap independent19.|
|2|D110 type1 model membership|54|Same real manager/entry storage as accepted D150 query, with type1 rather than2. No directcall provider. Source025 custommodel manager absent; R5 query adaptation low cost.|
|3|D2C0 load type1 model|94|Calls C650/281, NOT accepted C770 despite equal bodylength. C650 calls InstallA7BD0/81, NOT accepted A7C30. Therefore not a standalone94 closure. No025 custommodel implementation.|
|4|6C9B0 object/model preview raster|694|No full body found025/base. Raster/camera/world wrappers largely accepted. Missing six direct wrappers B6250/81,B6360/74,B6410/93,B6330/37,B4520/9,B62B0/57 plus native sleep/import and object-vtable identity. B6330 tailcalls B62F0/59 -> B1430/20, both pending. Of the three renderers this has the smallest constructor burden.|
|5|6C140 ped preview raster|634|No full preview body found025/base. Creates CPlayerPed via B0CE0/694;025 supplies model/ped-number constructor source skeleton, activebase lacks that overload’s reconstructed body. Other missing direct methods AFF50/113,B5790/72,9FD00/91,AE530/41,9FB20/138; polymorphic calls throughslots0/4/8 need true wrapper vtable/destructor identities. Greater semantic/ABI/allocation burden than objectpreview.|
|6|6C3C0 vehicle preview raster|686|No full preview body found025/base. CVehicle B83D0/1308 required;025 has vehicle constructor source, basevehicle.cpp constructor is TODO. Also missing B6360/74,B5790/72,9FD00/91,9FB20/138 and vtable calls. Vehicleconstructor reaches B42B0/65 plus native imports and multiple script descriptors. Highest immediate constructor cost.|

## Type1 custom model branch

D2C0 -> C650281 -> A7BD081 -> A7A8076,B1F10137,B45D036, with accepted LoadCustomModelTextureA7B60. Known transitive type1 additions already sum705 bytes including D2C094, before auditing all deeper calls/native virtuals. C650 path resolver can reuse the now-exact symbolic source structure only with its genuinely different installprovider; relabeling C770 would be wrong. Do not bypass this branch to accept PreviewUpdate.

## Transitive lower bound and value

Direct requested closure = PreviewUpdate513+PoolPass36+six missing directproviders2181 =2730 principal bytes. Merely adding presently identified unique second-level bodies gives a lower bound6393 principal bytes (includes type1 chain611, ped/shared methods1149, vehicle-specific1447, object additional456). This excludes EH, tables, any newly found deeper calls, vtable arrays and storage construction. It is an audit lower bound, not a ready batch or credible completion-time estimate. The graph can grow. Full model-info vtable targets/native imports and preview owner camera/world/light initialization must be identified; a pointerstore alone does not establish those objects.

Best near-term gain within this requested graph is73 independent bytes (B3DB0+D110), insufficient to open the549 outerbodies on its own. Object preview wrappers are a possible coherent preparatory family, but still do not close the ped/vehicle/type1 branches needed for the whole513. With the user’s unique-byte criterion, pursuing alreadyclosed RPC families likely yields more accepted bytes per integration cycle than this multi-constructor preview closure. No source written per request to avoid a large speculative batch.

025 value is concentrated in subordinate ped/vehicle constructor templates, not the six requested direct providers themselves. Those templates still need R5 layouts, script operands/native identities and whole-body matching; no transferbyte credit assigned from availability alone.
