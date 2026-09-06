
#include "ID3DXFontHook.h"
#include "../main.h"

// Reconstructed actual R5 globals and helpers; no new fake scratch allocation.
extern ID3DXSprite *r5FontActualSprite;
extern DWORD r5FontEmbeddedColor;
extern int r5FontCharacterIndex;
extern wchar_t r5FontWideText[20000];
extern char *r5FontOriginalText;
extern char *r5FontStrippedText;
extern void R5FontConvertAnsiToWide(char *, wchar_t *, int);
extern void RemoveColorEmbedsFromString(char *);
class R5FontSpriteProxy;
extern R5FontSpriteProxy r5FontSpriteProxy;

INT __stdcall ID3DXFontHook::DrawTextA(LPD3DXSPRITE pSprite, LPCSTR pString, INT Count, LPRECT pRect, DWORD Format, D3DCOLOR Color)
{
 if (pSprite)
 {
  r5FontActualSprite=pSprite;
  r5FontEmbeddedColor=Color;
  r5FontCharacterIndex=0;
  if (strlen(pString)>100000)
   return 0;
  strcpy(r5FontOriginalText,pString);
  strcpy(r5FontStrippedText,pString);
  R5FontConvertAnsiToWide(r5FontOriginalText,r5FontWideText,20000);
  RemoveColorEmbedsFromString(r5FontStrippedText);
  return m_pD3DFont->DrawTextA(reinterpret_cast<ID3DXSprite *>(&r5FontSpriteProxy),
    r5FontStrippedText,strlen(r5FontStrippedText),pRect,Format,Color);
 }
 else
  return m_pD3DFont->DrawTextA(NULL,pString,Count,pRect,Format,Color);
}

HRESULT __stdcall ID3DXFontHook::QueryInterface(REFIID iid, LPVOID *ppv)
{
	return m_pD3DFont->QueryInterface(iid, ppv);
}

ULONG __stdcall ID3DXFontHook::AddRef()
{
	return m_pD3DFont->AddRef();
}

ULONG __stdcall ID3DXFontHook::Release()
{
	return m_pD3DFont->Release();
}

HRESULT __stdcall ID3DXFontHook::GetDevice(LPDIRECT3DDEVICE9 *ppDevice)
{
	return m_pD3DFont->GetDevice(ppDevice);
}

HRESULT __stdcall ID3DXFontHook::GetDescA(D3DXFONT_DESCA *pDesc)
{
	return m_pD3DFont->GetDescA(pDesc);
}

HRESULT __stdcall ID3DXFontHook::GetDescW(D3DXFONT_DESCW *pDesc)
{
	return m_pD3DFont->GetDescW(pDesc);
}

BOOL __stdcall ID3DXFontHook::GetTextMetricsA(TEXTMETRICA *pTextMetrics)
{
	return m_pD3DFont->GetTextMetricsA(pTextMetrics);
}

BOOL __stdcall ID3DXFontHook::GetTextMetricsW(TEXTMETRICW *pTextMetrics)
{
	return m_pD3DFont->GetTextMetricsW(pTextMetrics);
}

HDC __stdcall ID3DXFontHook::GetDC()
{
	return m_pD3DFont->GetDC();
}

HRESULT __stdcall ID3DXFontHook::GetGlyphData(UINT Glyph, LPDIRECT3DTEXTURE9 *ppTexture, RECT *pBlackBox, POINT *pCellInc)
{
	return m_pD3DFont->GetGlyphData(Glyph, ppTexture, pBlackBox, pCellInc);
}

HRESULT __stdcall ID3DXFontHook::PreloadCharacters(UINT First, UINT Last)
{
	return m_pD3DFont->PreloadCharacters(First, Last);
}

HRESULT __stdcall ID3DXFontHook::PreloadGlyphs(UINT First, UINT Last)
{
	return m_pD3DFont->PreloadGlyphs(First, Last);
}

HRESULT __stdcall ID3DXFontHook::PreloadTextA(LPCSTR pString, INT Count)
{
	return m_pD3DFont->PreloadTextA(pString, Count);
}

HRESULT __stdcall ID3DXFontHook::PreloadTextW(LPCWSTR pString, INT Count)
{
	return m_pD3DFont->PreloadTextW(pString, Count);
}

INT __stdcall ID3DXFontHook::DrawTextW(LPD3DXSPRITE pSprite, LPCWSTR pString, INT Count, LPRECT pRect, DWORD Format, D3DCOLOR Color)
{
	return m_pD3DFont->DrawTextW(pSprite, pString, Count, pRect, Format, Color);
}

HRESULT __stdcall ID3DXFontHook::OnLostDevice()
{
	return m_pD3DFont->OnLostDevice();
}

HRESULT __stdcall ID3DXFontHook::OnResetDevice()
{
	return m_pD3DFont->OnResetDevice();
}

