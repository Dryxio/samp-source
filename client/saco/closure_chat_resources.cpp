// Whole R5 chat render-target owner setup, independent of Draw662.
#include "main.h"
extern CConfig *pConfig;
#pragma pack(push,1)
class R5ChatResourcesView
{
public:
 BYTE prefix[0x63A2];
 CFontRender *font;
 ID3DXSprite *sprite,*cachedSprite;
 IDirect3DDevice9 *device;
 int cacheEnabled;
 ID3DXRenderToSurface *renderToSurface;
 IDirect3DTexture9 *texture;
 IDirect3DSurface9 *surface;
 D3DDISPLAYMODE display;
 DWORD lastUpdateTick;
 int cacheValid,cacheDirty,lastScrollPosition,fontHeight,timestampWidth;
 void MeasureDimensions();
};
#pragma pack(pop)
void R5ChatResourcesView::MeasureDimensions()
{
 RECT rect;
 font->field_0->DrawTextA(NULL,"Y",-1,&rect,DT_CALCRECT|DT_SINGLELINE,0xFF000000);
 fontHeight=rect.bottom-rect.top;
 font->field_0->DrawTextA(NULL,"[19:58:34]",-1,&rect,DT_CALCRECT|DT_SINGLELINE,0xFF000000);
 timestampWidth=rect.right-rect.left;
}
void CChatWindow::CreateFonts()
{
 R5ChatResourcesView *owner=(R5ChatResourcesView*)this;
 owner->cacheEnabled=1;
 SAFE_RELEASE(owner->surface);
 SAFE_RELEASE(owner->texture);
 SAFE_RELEASE(owner->renderToSurface);
 owner->device->GetDisplayMode(0,&owner->display);
 if(pConfig->GetIntVariable("directmode"))
 {
  AddDebugMessage("ChatWindow: Using direct drawing mode.");
  owner->MeasureDimensions();
  owner->cacheEnabled=0;
  return;
 }
 HRESULT result;
 if(owner->display.Width<=1024)
  result=D3DXCreateTexture(owner->device,1024,512,1,D3DUSAGE_RENDERTARGET,D3DFMT_A8R8G8B8,D3DPOOL_DEFAULT,&owner->texture);
 else
  result=D3DXCreateTexture(owner->device,2048,1024,1,D3DUSAGE_RENDERTARGET,D3DFMT_A8R8G8B8,D3DPOOL_DEFAULT,&owner->texture);
 if(FAILED(result))
 {
  AddDebugMessage("ChatWindow: Can't create a render surface texture. Will use direct mode.");
  owner->cacheEnabled=0;
  return;
 }
 owner->texture->GetSurfaceLevel(0,&owner->surface);
 D3DSURFACE_DESC description;
 owner->surface->GetDesc(&description);
 result=D3DXCreateRenderToSurface(owner->device,description.Width,description.Height,description.Format,TRUE,D3DFMT_D16,&owner->renderToSurface);
 if(FAILED(result))
 {
  AddDebugMessage("ChatWindow: Can't create a render to surface. Will use direct mode.");
  owner->MeasureDimensions();
  owner->cacheEnabled=0;
  return;
 }
 owner->MeasureDimensions();
 owner->cacheValid=0;
 owner->cacheDirty=1;
}
