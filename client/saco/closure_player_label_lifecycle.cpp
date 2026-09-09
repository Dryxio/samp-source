// Pointer-only view shared with existing label reconstruction; never allocated.
#include "main.h"
class R5PlayerTagLabelView
{
public:
 IDirect3DDevice9 *device;
 IDirect3DStateBlock9 *states;
 ID3DXSprite *sprite;
 void BeginLabel();
 void EndLabel();
 void DrawLabel(D3DXVECTOR3 *position,char *name,DWORD color,float distance,bool showStatus,int status);
};
void R5PlayerTagLabelView::BeginLabel() { if(sprite) sprite->Begin(D3DXSPRITE_ALPHABLEND); }
void R5PlayerTagLabelView::EndLabel() { if(sprite) sprite->End(); }
