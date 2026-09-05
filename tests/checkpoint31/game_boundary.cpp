// Test-only game interface. Any invocation fails the native run.
#include <windows.h>
#include "../../vendor/upstream/saco/d3dhook/ID3DXFontHook.h"
class CGame;
CGame* pGame=0;
char* GetFontFace() { ExitProcess(90); return 0; }
int GetFontWeight() { ExitProcess(91); return 0; }
int GetUIFontSize() { ExitProcess(92); return 0; }
HRESULT __stdcall ID3DXFontHook::QueryInterface(REFIID iid, LPVOID *ppv) { ExitProcess(93); return 0; }
ULONG __stdcall ID3DXFontHook::AddRef() { ExitProcess(93); return 0; }
ULONG __stdcall ID3DXFontHook::Release() { ExitProcess(93); return 0; }
HRESULT __stdcall ID3DXFontHook::GetDevice(LPDIRECT3DDEVICE9 *ppDevice) { ExitProcess(93); return 0; }
HRESULT __stdcall ID3DXFontHook::GetDescA(D3DXFONT_DESCA *pDesc) { ExitProcess(93); return 0; }
HRESULT __stdcall ID3DXFontHook::GetDescW(D3DXFONT_DESCW *pDesc) { ExitProcess(93); return 0; }
BOOL __stdcall ID3DXFontHook::GetTextMetricsA(TEXTMETRICA *pTextMetrics) { ExitProcess(93); return 0; }
BOOL __stdcall ID3DXFontHook::GetTextMetricsW(TEXTMETRICW *pTextMetrics) { ExitProcess(93); return 0; }
HDC __stdcall ID3DXFontHook::GetDC() { ExitProcess(93); return 0; }
HRESULT __stdcall ID3DXFontHook::GetGlyphData(UINT Glyph, LPDIRECT3DTEXTURE9 *ppTexture, RECT *pBlackBox, POINT *pCellInc) { ExitProcess(93); return 0; }
HRESULT __stdcall ID3DXFontHook::PreloadCharacters(UINT First, UINT Last) { ExitProcess(93); return 0; }
HRESULT __stdcall ID3DXFontHook::PreloadGlyphs(UINT First, UINT Last) { ExitProcess(93); return 0; }
HRESULT __stdcall ID3DXFontHook::PreloadTextA(LPCSTR pString, INT Count) { ExitProcess(93); return 0; }
HRESULT __stdcall ID3DXFontHook::PreloadTextW(LPCWSTR pString, INT Count) { ExitProcess(93); return 0; }
INT __stdcall ID3DXFontHook::DrawTextA(LPD3DXSPRITE pSprite, LPCSTR pString, INT Count, LPRECT pRect, DWORD Format, D3DCOLOR Color) { ExitProcess(93); return 0; }
INT __stdcall ID3DXFontHook::DrawTextW(LPD3DXSPRITE pSprite, LPCWSTR pString, INT Count, LPRECT pRect, DWORD Format, D3DCOLOR Color) { ExitProcess(93); return 0; }
HRESULT __stdcall ID3DXFontHook::OnLostDevice() { ExitProcess(93); return 0; }
HRESULT __stdcall ID3DXFontHook::OnResetDevice() { ExitProcess(93); return 0; }
