// Complete 28-byte allocation established at C4D32 -> new(0x1c),6CCC0 ctor.
// Normal C++, all Init providers already accepted. No fictitious object instance.
#include "main.h"
#include "game/rwstuff.h"
class R5PreviewRenderer {
 RwCamera *camera; RwFrame *frame; RpLight *light; RwTexture *texture;
 RwRaster *raster; RwRaster *zBuffer; DWORD unknown18;
public:
 R5PreviewRenderer();
 bool Initialize();
};
typedef char r5_preview_renderer_size28[(sizeof(R5PreviewRenderer)==28)?1:-1];
R5PreviewRenderer::R5PreviewRenderer() {
 camera=0;frame=0;texture=0;unknown18=0;light=0;
 Initialize();
}
bool R5PreviewRenderer::Initialize() {
 light=RpLightCreate(2);
 if(!light)return false;
 RwRGBAReal color={1.0f,1.0f,1.0f,1.0f};
 RpLightSetColor(light,&color);
 raster=RwRasterCreate(256,256,0,0x505);
 zBuffer=RwRasterCreate(256,256,0,1);
 camera=RwCameraCreate();
 frame=RwFrameCreate();
 VECTOR position={0.0f,0.0f,50.0f};
 RwFrameTranslate(frame,&position,1);
 RwFrameRotate(frame,0,90.0f);
 if(!camera || !frame || !raster || !zBuffer)return false;
 SetCameraFrameAndZBuffer(camera,raster,zBuffer);
 RwObjectHasFrameSetFrame(camera,frame);
 RwCameraSetClipPlane(camera,300.0f,0.01f);
 VECTOR2D viewWindow={0.5f,0.5f};
 RwCameraSetViewWindow(camera,&viewWindow);
 RwCameraSetProjection(camera,1);
 RpWorldAddCamera(camera);
 texture=RwTextureCreate(raster);
 return true;
}
