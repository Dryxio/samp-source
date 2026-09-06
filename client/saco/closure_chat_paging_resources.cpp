#include "main.h"
#include <stddef.h>
#pragma pack(push,1)
class R5ChatPagingView
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
 void ConfigureScrollBar();
 void SetPageSize(int lines);
 void ScrollWheel(int notches);
 void ReleaseSurfaces();
 void FilterInvalidChars(char *text);
};
#pragma pack(pop)
typedef char R5ChatPagingSize[(sizeof(R5ChatPagingView)==0x63EA)?1:-1];
void R5ChatPagingView::ConfigureScrollBar()
{
	if(scrollbar) {
		scrollbar->SetLocation(10,40);
		scrollbar->SetSize(20,((fontHeight+1)*pageSize)-60);
		scrollbar->SetTrackRange(1,MAX_MESSAGES);
		scrollbar->SetPageSize(pageSize);
		scrollbar->SetTrackPos(MAX_MESSAGES-pageSize);
	}
}
void R5ChatPagingView::SetPageSize(int lines)
{
 if(lines>=10 && lines<=100) {
  pageSize=lines;
  ConfigureScrollBar();
  cacheDirty=1;
 }
}
void R5ChatPagingView::ScrollWheel(int notches)
{
 if(displayMode && scrollbar) {
  UINT lines;
  SystemParametersInfo(SPI_GETWHEELSCROLLLINES,0,&lines,0);
  scrollbar->Scroll(-(notches*lines));
 }
}
void R5ChatPagingView::ReleaseSurfaces()
{
 if(surface) { surface->Release();surface=NULL; }
 if(texture) { texture->Release();texture=NULL; }
 if(renderToSurface) { renderToSurface->Release();renderToSurface=NULL; }
}
void R5ChatPagingView::FilterInvalidChars(PCHAR szString)
{
	while(*szString) {
		if(*szString > 0 && *szString < ' ') {
			*szString = ' ';
		}
		szString++;
	}
}
