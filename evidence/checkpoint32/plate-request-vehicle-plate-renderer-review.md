# Vehicle plate renderer: bounded first C++ proposal

525 potential code bytes, not compiled or accepted. Whole function ranges and original SHA-256/PE relocations are in vehicle-plate-original-manifest.json. No claimed 0.2.5 direct source credit: base unkclass13.cpp supplies lifecycle; RenderTexture is R5 reconstruction. No raw bytes, symbolic assembly, padding, or fake allocations.

| Real identity | RVA | Bytes |
|---|---:|---:|
| DeleteDeviceObjects | 6D7B0 | 73 |
| RestoreDeviceObjects | 6D800 | 118 |
| RenderTexture | 6D880 | 295 |
| Complete owner constructor | 6D9B0 | 34 |
| Destructor tailcall | 6D9E0 | 5 |

Full owner is 36 bytes: device0, renderToSurface4, renderTexture8, renderSurfaceC, complete D3DDISPLAYMODE10..1F, defaultTexture20. Source has sizeof/offset assertions. No vtable, no unknown gaps, no allocation of a pointer-only view. Constructor zeroes four pointer fields, preserves full native display structure initialized by Restore. Startup DoInitStuff original allocates 0x24 at C4CCC/C4CD3 then invokes constructor C4CF4; the corresponding singleton is 26EBA0, read by the actual vehicle plate cache at B81D0. This independent caller checks vehicle texture8F, checks text93, calls renderer6D880 at B81D7, then stores returned texture8F. Do not include unrelated 6D9F0/8 (different this+65 owner).

Dependencies: actual CFontRender::RenderText(char*,RECT,DWORD) 6B650, pDefaultFont26EB9C already owned. D3DXCreateTexture C5C6E -> E5520; D3DXCreateRenderToSurface C5C68 -> E5524; newly required D3DXLoadSurfaceFromSurface C5C86 -> E5510. Import descriptor directly read from R5 confirms all three are d3dx9_25.dll; use library=d3dx9.lib. Import thunks get zero source coverage. Constructor/cleanup internal calls use actual new class methods. No store for singleton is needed by these five functions, so existing pUnkClass13 global definition remains untouched.

Real COM stdcall sites: IUnknown Release slot8; Device GetDisplayMode20 and ClearAC; Texture SetAutoGenFilterType38 and GetSurfaceLevel48; Surface GetDesc30; RenderToSurface BeginScene14, EndScene18. No handcrafted bridge or guessed native vtable.

Complete literals: render target128x32, level1, usage1, display.Format, pool0, depth format80; output texture64x32, level1, usage0, display.Format, pool0. Background FFBEB6A8; foreground EE444470; immediate float1.0f (3F800000); text RECT 0,3,128,32; texture filter2; surface copy filter3. All embedded immediate data, no external string or float region. No new EH or global data. The two pDefaultFont reads are actual original absolute relocations; all remaining calls/imports must be verified once compiled. Unreferenced header sentinel emissions must not be credited.

Failure behavior preserved: return NULL if any required pointer absent or output texture creation fails. Ignore BeginScene, Clear, font draw, EndScene, GetSurfaceLevel and copy return values; no invented cleanup/repair path. Release only the output surface, return output texture to caller.

Proposed source: closure_vehicle_plate_renderer_proposed.cpp. First probe Ob1, matching existing base source visibility (constructor/destructor precede the later out-of-line lifecycle definitions). If compiler differs, diagnose actual COFF before altering source. Root alone may compile/integrate. Cache wrapper B81A0/69 is a separate future extension and is not proposed/countable here.
