# Vehicle preview — normal C++ proposal, not yet compiled

6C3C0/686 plus actual EH21 E3980 =707 unique code bytes after accepted223896.
No overlap with constructor/lifetime2180 candidates, or Entityqueries278.
Actual direct calls are all accepted except CVehicleB83D0 ctor in current probe.
Virtual Add and scalar deleting destructor require whole genuine CVehicle lifetime
acceptance; no assumed unimplemented vtable slots or fake0xB8 allocation.

closure_vehicle_preview_render_proposed.cpp mirrors confirmed ped renderer forms
while retaining original vehicle differences: carriage570→locomotive538 and
569→537, spawn atZ50, no separate model-setter, collision-based camera distance,
optional int colors (-1 sentinel), one Add and virtual delete without explicit
Remove, actual constructor handles releasing model on destruction.
Signature six stackargs: model int, backgroundDWORD, rotation VECTOR*, zoomfloat,
color1int,color2int; thiscall ret24. True manager pointer-only view observes
camera0,light8,zbuffer14; no instances of that partial view are created.

Actual float literals read from original PE:
EA638 000080bf=-1.0f; EA634 00002040=2.5f; EA630 0000b0c0=-5.5f;
E5940 zero. Immediate50.0f bits42480000; radius*2 compiled FADD itself.
Default distance=(-1.0f-2.0f*radius)*zoom, subtype4 override=-5.5f-2.5f*radius
explicitly excludes zoom (original behavior). Do not normalize this distinction.

New genuine BSS26EBE0/4 r5VehiclePreviewRenderActive has PErefs6C60A/6C619
(write1/write0 bracketing EntityRender9FB20), C3D31 (read), C4F76 (initialization).
C3D30 hook reads flag and substitutes0 for D3DRS_FOGENABLE28 during rendering.
Its native COM SetRenderState calls are vtableE4, pushed arguments; this independent
consumer is not called by the reconstructed renderer and receives no coverage.
Only store4 is proposed in separate minimal include TU, no new hook implementation.

EH E3980/21: operator-delete cleanup11 then CxxFrameHandler thunk10E398B,
FuncInfoFB850, complete xdataFB848/36. No tables or forced padding.
Provider/literal/relocation manifest: vehicle-preview-manifest.json.
Actual full reference capture: vehicle-preview-original.asm.
0.2.5 has no preview renderer implementation: direct source025 credit0; its real
vehicle constructor and lifetime now provide the reusable missing foundation.
