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
#include <stdlib.h>
#include <string.h>
extern CChatWindow *pChatWindow;
extern CConfig *pConfig;
extern CFontRender *pDefaultFont;
extern CDeathWindow *pDeathWindow;
extern void RefreshApplicationUiFonts();
void cmdFontSize(PCHAR szCmd)
{
 int size;
 if(strlen(szCmd) && (size=atoi(szCmd))>=-3 && size<=5) {
  if(pConfig) pConfig->SetIntVariable("fontsize",size);
  if(pDefaultFont) pDefaultFont->CreateFonts();
  if(pChatWindow) {
   pChatWindow->CreateFonts();
   ((R5ChatPagingView*)pChatWindow)->ConfigureScrollBar();
  }
  if(pDeathWindow) pDeathWindow->CreateFonts();
  RefreshApplicationUiFonts();
 } else {
  if(pChatWindow) pChatWindow->AddDebugMessage("Valid fontsize: -3 to 5");
 }
}
