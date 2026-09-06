// R5 vehicle registration-plate texture renderer. Complete 36-byte owner.
// Lifecycle adapted from base unkclass13.cpp; RenderTexture reconstructed R5.
#include "main.h"
#include <stddef.h>
extern CFontRender *pDefaultFont;
#pragma pack(push,1)
class R5VehiclePlateRenderer {
public:
 IDirect3DDevice9 *device;
 ID3DXRenderToSurface *renderToSurface;
 IDirect3DTexture9 *renderTexture;
 IDirect3DSurface9 *renderSurface;
 D3DDISPLAYMODE displayMode;
 IDirect3DTexture9 *defaultTexture;
 __declspec(dllexport) R5VehiclePlateRenderer(IDirect3DDevice9 *d);
 __declspec(dllexport) ~R5VehiclePlateRenderer();
 void DeleteDeviceObjects();
 void RestoreDeviceObjects();
 IDirect3DTexture9 *RenderTexture(char *plateText);
};
#pragma pack(pop)
typedef char PlateOwnerSize[(sizeof(R5VehiclePlateRenderer)==36)?1:-1];
typedef char PlateDisplayOffset[(offsetof(R5VehiclePlateRenderer,displayMode)==16)?1:-1];
typedef char PlateDefaultOffset[(offsetof(R5VehiclePlateRenderer,defaultTexture)==32)?1:-1];
R5VehiclePlateRenderer::R5VehiclePlateRenderer(IDirect3DDevice9 *d) {
 defaultTexture=0; renderSurface=0; renderTexture=0; renderToSurface=0; device=d;
 RestoreDeviceObjects();
}
R5VehiclePlateRenderer::~R5VehiclePlateRenderer() { DeleteDeviceObjects(); }
void R5VehiclePlateRenderer::DeleteDeviceObjects() {
 SAFE_RELEASE(renderSurface);
 SAFE_RELEASE(renderTexture);
 SAFE_RELEASE(renderToSurface);
 SAFE_RELEASE(defaultTexture);
}
void R5VehiclePlateRenderer::RestoreDeviceObjects() {
 device->GetDisplayMode(0,&displayMode);
 HRESULT hr=D3DXCreateTexture(device,128,32,1,D3DUSAGE_RENDERTARGET,
  displayMode.Format,D3DPOOL_DEFAULT,&renderTexture);
 if(SUCCEEDED(hr)) {
  D3DSURFACE_DESC desc;
  renderTexture->GetSurfaceLevel(0,&renderSurface);
  renderSurface->GetDesc(&desc);
  D3DXCreateRenderToSurface(device,desc.Width,desc.Height,desc.Format,
   TRUE,D3DFMT_D16,&renderToSurface);
 }
}
IDirect3DTexture9 *R5VehiclePlateRenderer::RenderTexture(char *plateText) {
 if(!device || !pDefaultFont || !renderToSurface || !renderTexture || !renderSurface)
  return NULL;
 IDirect3DTexture9 *result;
 HRESULT hr=D3DXCreateTexture(device,64,32,1,0,displayMode.Format,D3DPOOL_DEFAULT,&result);
 if(FAILED(hr)) return NULL;
 renderToSurface->BeginScene(renderSurface,NULL);
 device->Clear(0,NULL,D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER,0xFFBEB6A8,1.0f,0);
 RECT textRect={0,3,128,32};
 pDefaultFont->RenderText(plateText,textRect,0xEE444470);
 renderToSurface->EndScene(0);
 result->SetAutoGenFilterType(D3DTEXF_LINEAR);
 IDirect3DSurface9 *outputSurface;
 result->GetSurfaceLevel(0,&outputSurface);
 D3DXLoadSurfaceFromSurface(outputSurface,NULL,NULL,renderSurface,NULL,NULL,(DWORD)textRect.top,0);
 outputSurface->Release();
 return result;
}
