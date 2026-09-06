// R5 6C3C0/686, normal C++; requires real CVehicle ctor/vtable/lifetime closure.
#include "main.h"
#include "game/rwstuff.h"
#include <string.h>
void RotatePreviewMatrix(MATRIX4X4*,int,float);
void CallRwRenderStateSet(int,int);
float GetModelCollisionRadius(int model);
extern DWORD r5VehiclePreviewRenderActive;
struct R5PreviewNativeEntityView {
 DWORD vtable; BYTE unknown4[0x14]; void *rwObject; BYTE unknown1C[6]; WORD model;
};
class R5PreviewEntityRenderView {
 BYTE unknown0[0x40]; R5PreviewNativeEntityView *entity;
public: void Render();
};
// Pointer-only manager view, shared member layout camera0/light8/zbuffer14.
class R5VehiclePreviewRenderView {
 RwCamera *camera; BYTE unknown4[4]; RpLight *light;
 BYTE unknownC[8]; RwRaster *zBuffer;
public:
 RwTexture *RenderVehicle(int model,DWORD background,VECTOR *rotation,float zoom,int color1,int color2);
};
RwTexture *R5VehiclePreviewRenderView::RenderVehicle(int model,DWORD background,VECTOR *rotation,float zoom,int color1,int color2) {
 RwRaster *raster=RwRasterCreate(256,256,0,0x505);
 RwTexture *texture=RwTextureCreate(raster);
 if(model==570)model=538;
 else if(model==569)model=537;
 CVehicle *vehicle=new CVehicle(model,0.0f,0.0f,50.0f,0.0f,FALSE,0);
 if(!raster || !texture || !vehicle)return NULL;
 vehicle->SetGravityProcessing(FALSE);
 vehicle->SetCollisionChecking(FALSE);
 float distance=(-1.0f-GetModelCollisionRadius(model)*2.0f)*zoom;
 if(vehicle->GetVehicleSubtype()==4)distance=-5.5f-GetModelCollisionRadius(model)*2.5f;
 vehicle->TeleportTo(0.0f,distance,50.0f);
 if(color1!=-1 && color2!=-1)vehicle->SetColor(color1,color2);
 MATRIX4X4 matrix;
 memset(&matrix,0,sizeof(matrix));
 vehicle->GetMatrix(&matrix);
 if(rotation->X!=0.0f)RotatePreviewMatrix(&matrix,0,rotation->X);
 if(rotation->Y!=0.0f)RotatePreviewMatrix(&matrix,1,rotation->Y);
 if(rotation->Z!=0.0f)RotatePreviewMatrix(&matrix,2,rotation->Z);
 vehicle->SetMatrixAndUpdate(matrix);
 SetCameraFrameAndZBuffer(camera,raster,zBuffer);
 CVisibilityPlugins_SetRenderWareCamera(camera);
 RwCameraClear(camera,(RwRGBA*)&background,3);
 RwCameraBeginUpdate(camera);
 RpWorldAddLight(light);
 CallRwRenderStateSet(6,1);CallRwRenderStateSet(8,1);
 CallRwRenderStateSet(7,2);CallRwRenderStateSet(14,0);
 vehicle->Add();
 r5VehiclePreviewRenderActive=1;
 ((R5PreviewEntityRenderView*)vehicle)->Render();
 r5VehiclePreviewRenderActive=0;
 RwCameraEndUpdate(camera);
 RpWorldRemoveLight(light);
 delete vehicle;
 return texture;
}
