#include "main.h"
// Pointer-only access view: no extra fields, no object construction or new vtable.
// A distinct out-of-line method preserves the original 670E0 body despite
// another TU emitting the shorter same-name inline SDK specialization.
class R5ChatScrollBarNavigationView : public CDXUTScrollBar
{
public:
 void R5SetTrackPos(int position);
};
typedef char R5ChatScrollBarNavigationSizeCheck[
 sizeof(R5ChatScrollBarNavigationView)==sizeof(CDXUTScrollBar) ? 1 : -1];

#pragma pack(push,1)
class R5ChatPageNavigationView
{
public:
 int pageSize;
 BYTE gap4[4];
 int displayMode;
 BYTE timestamps;
 BYTE gapD[0x11E-0xD];
 CDXUTScrollBar *scrollbar;
 DWORD chatColor,infoColor,debugColor;
 int inputLineY;
 CHAT_WINDOW_ENTRY entries[100];
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
 void PageUp();
 void PageDown();
};
#pragma pack(pop)
extern CGame *pGame;
extern CCmdWindow *pCmdWindow;
class R5ChatGameMenuView { public: BOOL IsMenuActive(); };
class R5ChatInputQueryView { public: IDirect3DDevice9 *device; CDXUTDialog *dialog; CDXUTIMEEditBox *edit; bool IsImeInteractionActive(); };
void R5ChatPageNavigationView::PageUp()
{
 if(!displayMode || !scrollbar) return;
 if(((R5ChatGameMenuView*)pGame)->IsMenuActive()) return;
 if(((R5ChatInputQueryView*)pCmdWindow)->IsImeInteractionActive()) return;
 int position=scrollbar->GetTrackPos()-pageSize;
 if(position<1) position=1;
 ((R5ChatScrollBarNavigationView*)scrollbar)->R5SetTrackPos(position);
}
void R5ChatPageNavigationView::PageDown()
{
 if(!displayMode || !scrollbar) return;
 if(((R5ChatGameMenuView*)pGame)->IsMenuActive()) return;
 if(((R5ChatInputQueryView*)pCmdWindow)->IsImeInteractionActive()) return;
 int position=scrollbar->GetTrackPos();
 if(position==1) position=pageSize;
 else position+=pageSize;
 if(position>100) position=100;
 ((R5ChatScrollBarNavigationView*)scrollbar)->R5SetTrackPos(position);
}

void R5ChatScrollBarNavigationView::R5SetTrackPos(int position)
{
 m_nPosition=position;
 Cap();
 UpdateThumbRect();
}
