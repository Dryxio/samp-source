# Independent Chat resources536

Whole source proposal closure_chat_resources_proposed.cpp: real existing
CChatWindow::CreateFonts681D0/419 plus pointer-only MeasureDimensions67120/117.
No header change. Existing CreateFonts declaration already present; root must
keep this sole implementation, replacing TODO ownership if needed. No fake
full-class allocation. Private fields accessed through measured packed view,
with existing offsets63A2..63E6, identical resource roles to previous renderer.

CreateFonts is misleading inherited base name: actual body recreates cached
render texture/surface and measures font; no new font is constructed here.
Original first flagcacheEnabled1, release surface→texture→renderToSurface,
GetDisplayMode. Config directmode disables cache after measurement. Allocation
texture fails: disable cache and return WITHOUT measurement (preserve). Render
surface fails: measure then disable cache. Success measure,valid0/dirty1.
Texture widths unsigned <=1024 test; sizes1024x512 or2048x1024; levels1,
usageRENDERTARGET1,formatA8R8G8B8=21,poolDEFAULT0. DepthformatD16=80.

Native COM slots: GetDisplayMode20,Release8,TextureGetSurfaceLevel48,
SurfaceGetDesc30, FontDrawTextA38. Actual constructor-owned fontfield0 used.
Measurements 'Y' and '[19:58:34]',flagsCALCRECT|SINGLELINE420,opaqueblack.
Header's trailing gap63E6 now observed timestampWidth; fontHeight63E2.

Real direct providers already covered configGetIntVariable65E10,
AddDebugMessage680F0 and configglobal26EB7C. Own new helper67120. No Draw662,
no parked preview or sprite dependency. Ob1 preserves helper call boundary.

Original import directory independently verified:
D3DXCreateTexture thunkC5C6E → IATE5520, d3dx9_25.dll.
D3DXCreateRenderToSurface thunkC5C68 → IATE5524, same module.
Both pinned D3DX stdcall imports, zero source-byte credit for thunks.
Complete six string sections with lengths and SHA in original manifest; Y2
already accepted, other five need source-owned matching full literal sections.
No floating-point data, new runtime globals, EH or tables expected.

Direct025 credit0. Base CreateFonts is TODO; reconstruction from R5 APIs and
confirmed full views. The old025 constructor font measurement supplies general
context but not this rendering-target setup implementation.
