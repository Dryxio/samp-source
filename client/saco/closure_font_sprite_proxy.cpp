#include <d3d9.h>
#include <d3dx9.h>
// Actual global sprite currently supplied to ID3DXFontHook::DrawTextA.
extern ID3DXSprite *r5FontActualSprite;
extern DWORD r5FontEmbeddedColor;
extern void R5FontConsumeWideColorEmbeds();

// Complete actual ID3DXSprite implementation, 14 COM slots, vptr only.
// No fake vtable array, absent lifetime methods, or fabricated allocation view.
class R5FontSpriteProxy : public ID3DXSprite
{
public:
 HRESULT __stdcall QueryInterface(REFIID iid, void **ppv);
 ULONG __stdcall AddRef();
 ULONG __stdcall Release();
 HRESULT __stdcall GetDevice(IDirect3DDevice9 **device);
 HRESULT __stdcall GetTransform(D3DXMATRIX *transform);
 HRESULT __stdcall SetTransform(const D3DXMATRIX *transform);
 HRESULT __stdcall SetWorldViewRH(const D3DXMATRIX *world, const D3DXMATRIX *view);
 HRESULT __stdcall SetWorldViewLH(const D3DXMATRIX *world, const D3DXMATRIX *view);
 HRESULT __stdcall Begin(DWORD flags);
 HRESULT __stdcall Draw(IDirect3DTexture9 *texture, const RECT *rect,
                        const D3DXVECTOR3 *center, const D3DXVECTOR3 *position,
                        D3DCOLOR color);
 HRESULT __stdcall Flush();
 HRESULT __stdcall End();
 HRESULT __stdcall OnLostDevice();
 HRESULT __stdcall OnResetDevice();
};
typedef char R5FontSpriteProxyWholeSize[sizeof(R5FontSpriteProxy)==4 ? 1 : -1];

HRESULT __stdcall R5FontSpriteProxy::QueryInterface(REFIID iid, void **ppv)
{ return r5FontActualSprite->QueryInterface(iid, ppv); }
ULONG __stdcall R5FontSpriteProxy::AddRef()
{ return r5FontActualSprite->AddRef(); }
ULONG __stdcall R5FontSpriteProxy::Release()
{ return r5FontActualSprite->Release(); }
HRESULT __stdcall R5FontSpriteProxy::GetDevice(IDirect3DDevice9 **device)
{ return r5FontActualSprite->GetDevice(device); }
HRESULT __stdcall R5FontSpriteProxy::GetTransform(D3DXMATRIX *transform)
{ return r5FontActualSprite->GetTransform(transform); }
HRESULT __stdcall R5FontSpriteProxy::SetTransform(const D3DXMATRIX *transform)
{ return r5FontActualSprite->SetTransform(transform); }
HRESULT __stdcall R5FontSpriteProxy::SetWorldViewRH(const D3DXMATRIX *world, const D3DXMATRIX *view)
{ return r5FontActualSprite->SetWorldViewRH(world,view); }
HRESULT __stdcall R5FontSpriteProxy::SetWorldViewLH(const D3DXMATRIX *world, const D3DXMATRIX *view)
{ return r5FontActualSprite->SetWorldViewLH(world,view); }
HRESULT __stdcall R5FontSpriteProxy::Begin(DWORD flags)
{ return r5FontActualSprite->Begin(flags); }
HRESULT __stdcall R5FontSpriteProxy::Draw(IDirect3DTexture9 *texture, const RECT *rect,
 const D3DXVECTOR3 *center, const D3DXVECTOR3 *position, D3DCOLOR color)
{
 R5FontConsumeWideColorEmbeds();
 return r5FontActualSprite->Draw(texture,rect,center,position,r5FontEmbeddedColor);
}
HRESULT __stdcall R5FontSpriteProxy::Flush()
{ return r5FontActualSprite->Flush(); }
HRESULT __stdcall R5FontSpriteProxy::End()
{ return r5FontActualSprite->End(); }
HRESULT __stdcall R5FontSpriteProxy::OnLostDevice()
{ return r5FontActualSprite->OnLostDevice(); }
HRESULT __stdcall R5FontSpriteProxy::OnResetDevice()
{ return r5FontActualSprite->OnResetDevice(); }

// Real initialized static object102BF0 -> compiler-produced VFT EA744/56.
// Probe must establish its complete initializer shape, not substitute a blob.
R5FontSpriteProxy r5FontSpriteProxy;
