# Health / matrix hook independent preflight

PASS, static preflight only: health Ob2 cp32-health-ob2-trial2 and hook/stores
Ob1 cp32-health-network-trial1. Eight complete functions, 1972 unique source
bytes against accepted reliability-preview 238956. All 155 fixups compared,
including complete PE relocation sets in both directions; 26 symbol bindings
checked against accepted real providers or new owners. No exception tables,
handlers, compiler vtables, or extra homologue helpers claimed. Ob2 genuinely
inlines Restore into Begin246; Draw1456 stays exact without a source alteration.

Integration manifest: player-tag-health-reviewed-manifest.json, format matching
TEA regions. 27 regions: eight code bodies, three complete initialized80-byte
vertex arrays, three complete64-byte BSS matrices, thirteen complete float4
literal sections. Every region records unit, section, anchor, RVA, full SHA256,
and every fixup's symbol/site_rva/target_va/kind. Existing globals and functions
retain their owners from reliability-preview-contract; no replacement provider.
Matrices are genuine zero-filled virtual-tail storage, with hook78 supplying
actual VIEW2/PROJECTION3/WORLD256 producers. All literal payloads are checked
against their actual four bytes, not merely inferred reference addresses.

Original PE import directory independently resolves IAT100E5514 to
`d3dx9_25.dll!D3DXVec3Project`; C5C80 is its JMP thunk with PE relocation C5C82.
The original SDK declares WINAPI, six pointer arguments, D3DXVECTOR3* result:
out/vector/viewport/projection/view/world. Matches the real Draw call ordering.
Pinned d3dx9.lib digest comes from existing sdk/private-input contract; root's
link must verify its actual import provider. Thunk contributes ZERO source
bytes, neither six additional accepted bytes nor proprietary imported code.

Actual CNewPlayerTags size12: device0, stateblock4, sprite8. No synthetic class
allocation or full hook vtable coverage. Source destructor deliberately uses
ordinary delete on sprite after StateBlock Release; original has the same
behavior. Keep it, rather than silently converting to COM Release.

COM slots from original calls and bundled D3D9 headers:
Device: GetViewport C0, CreateStateBlock EC, SetRenderState E4,
SetTextureStageState 10C, SetVertexShader170, SetPixelShader1AC,
SetFVF164, DrawPrimitiveUP14C; hook SetTransformB0.
StateBlock: Release8, Capture10, Apply14.
Sprite: Begin20, End2C, OnLostDevice30, OnResetDevice34.
These are genuine interface pointers received/created through the real APIs.
Original call sites include Restore6D6A2 and inline Begin6D6D8 for EC;
Draw6D0E3 for C0, six DrawPrimitiveUP calls; hook748C2 for B0.

D3DXMatrixIdentity59 is an already accepted owner; empty constructors3 and
D3DXMATRIX conversion22 are unused/homologous extra emissions, zero new credit.
Direct 0.2.5 attribution is ZERO for every body: these are unchanged existing
base implementations split into actual owner TUs, including SetTransform.
