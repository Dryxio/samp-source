// R5 6C140/634, real CPlayerPed allocation/lifetime; requires ctor694 closure.
#include "main.h"
#include "game/rwstuff.h"
#include <string.h>
extern CGame *pGame;
void RotatePreviewMatrix(MATRIX4X4*,int,float);
void CallRwRenderStateSet(int,int);
WORD __stdcall GetModelReferenceCount(int);
class RPCGameModelReleaseView { public: void RemoveModel(int model,BYTE force); };
class R5PreviewPedModelView {
    BYTE unknown0[0x44]; DWORD gtaId; BYTE unknown48[0x25C]; PED_TYPE *ped;
public: void SetModelIndex(UINT model);
};
class R5PreviewPedAnimationView {
    BYTE unknown0[0x2A4]; PED_TYPE *ped;
public: void UpdateAnimations(float step,int onScreen);
};
struct R5PreviewNativeEntityView {
    DWORD vtable; BYTE unknown4[0x14]; void *rwObject; BYTE unknown1C[6]; WORD model;
};
class R5PreviewEntityRenderView {
    BYTE unknown0[0x40]; R5PreviewNativeEntityView *entity;
public: void Render();
};
class R5PedPreviewRenderView {
    RwCamera *camera; BYTE unknown4[4]; RpLight *light;
    BYTE unknownC[8]; RwRaster *zBuffer;
public:
    RwTexture *RenderPed(int model,DWORD background,VECTOR *rotation,float zoom);
};
RwTexture *R5PedPreviewRenderView::RenderPed(int model,DWORD background,VECTOR *rotation,float zoom)
{
    RwRaster *raster=RwRasterCreate(256,256,0,0x505);
    RwTexture *texture=RwTextureCreate(raster);
    CPlayerPed *ped=new CPlayerPed(208,0,0.0f,0.0f,0.0f,0.0f);
    if(!raster || !texture || !ped) return NULL;
    float height=50.05f;
    if(model==162) height+=0.1f;
    ped->TeleportTo(0.0f,-2.25f*zoom,height);
    ((R5PreviewPedModelView*)ped)->SetModelIndex(model);
    ped->SetGravityProcessing(FALSE);
    ped->SetCollisionChecking(FALSE);
    MATRIX4X4 matrix;
    memset(&matrix,0,sizeof(matrix));
    ped->GetMatrix(&matrix);
    if(rotation->X!=0.0f) RotatePreviewMatrix(&matrix,0,rotation->X);
    if(rotation->Y!=0.0f) RotatePreviewMatrix(&matrix,1,rotation->Y);
    if(rotation->Z!=0.0f) RotatePreviewMatrix(&matrix,2,rotation->Z);
    ped->SetMatrixAndUpdate(matrix);
    SetCameraFrameAndZBuffer(camera,raster,zBuffer);
    CVisibilityPlugins_SetRenderWareCamera(camera);
    RwCameraClear(camera,(RwRGBA*)&background,3);
    RwCameraBeginUpdate(camera);
    RpWorldAddLight(light);
    CallRwRenderStateSet(6,1);
    CallRwRenderStateSet(8,1);
    CallRwRenderStateSet(7,2);
    CallRwRenderStateSet(14,0);
    ped->Add();
    ((R5PreviewPedAnimationView*)ped)->UpdateAnimations(100.0f,1);
    ((R5PreviewEntityRenderView*)ped)->Render();
    RwCameraEndUpdate(camera);
    RpWorldRemoveLight(light);
    ped->Remove();
    delete ped;
    if(!GetModelReferenceCount(model)) ((RPCGameModelReleaseView*)pGame)->RemoveModel(model,0);
    return texture;
}
