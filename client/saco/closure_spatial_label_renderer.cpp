// Complete original label owner, proven startup allocation8. No view allocation.
#include "main.h"
#include <stddef.h>
extern CFontRender *pDefaultFont;
extern D3DXMATRIX matView,matProj;
extern const SCRIPT_COMMAND r5LabelLineOfSight;
class R5SpatialLabelRenderer {
public:
 IDirect3DDevice9 *device;
 ID3DXSprite *sprite;
 __declspec(dllexport) R5SpatialLabelRenderer(IDirect3DDevice9 *d);
 __declspec(dllexport) ~R5SpatialLabelRenderer();
 void OnLostDevice();
 void OnResetDevice();
 BOOL IsLineOfSightClear(float x,float y,float z);
 void Begin();
 void End();
 void Draw(D3DXVECTOR3 *position,char *text,DWORD color,BOOL shadow,bool testLOS);
};
typedef char SpatialLabelSize[(sizeof(R5SpatialLabelRenderer)==8)?1:-1];
typedef char SpatialLabelSpriteOffset[(offsetof(R5SpatialLabelRenderer,sprite)==4)?1:-1];
R5SpatialLabelRenderer::R5SpatialLabelRenderer(IDirect3DDevice9 *d) {
 device=d;
 D3DXCreateSprite(d,&sprite);
}
R5SpatialLabelRenderer::~R5SpatialLabelRenderer() { SAFE_DELETE(sprite); }
void R5SpatialLabelRenderer::OnLostDevice() { if(sprite) sprite->OnLostDevice(); }
void R5SpatialLabelRenderer::OnResetDevice() { if(sprite) sprite->OnResetDevice(); }
BOOL R5SpatialLabelRenderer::IsLineOfSightClear(float x,float y,float z) {
 CAMERA_AIM *aim=GameGetInternalAim();
 if(aim && ScriptCommand(&r5LabelLineOfSight,x,y,z,
  aim->pos1x,aim->pos1y,aim->pos1z,1,0,0,1,0)) return TRUE;
 return FALSE;
}
void R5SpatialLabelRenderer::Begin() { if(sprite) sprite->Begin(D3DXSPRITE_ALPHABLEND); }
void R5SpatialLabelRenderer::End() { if(sprite) sprite->End(); }
void R5SpatialLabelRenderer::Draw(D3DXVECTOR3 *position,char *text,DWORD color,BOOL shadow,bool testLOS) {
 if(!device) return;
 if(testLOS && !IsLineOfSightClear(position->x,position->y,position->z)) return;
 D3DVIEWPORT9 viewport;
 device->GetViewport(&viewport);
 D3DXMATRIX world;
 D3DXMatrixIdentity(&world);
 D3DXVECTOR3 projected;
 D3DXVec3Project(&projected,position,&viewport,&matProj,&matView,&world);
 if(projected.z>1.0f) return;
 RECT rect;
 rect.left=(LONG)projected.x;
 rect.top=(LONG)projected.y;
 rect.right=rect.left+1;
 rect.bottom=rect.top+1;
 pDefaultFont->RenderText(sprite,text,rect,0x101,color,shadow);
}
