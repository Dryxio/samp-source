#include "main.h"
#include <stdlib.h>
#include <string.h>
extern CGame *pGame;
extern CChatWindow *pChatWindow;
extern CNetGame *pNetGame;
extern CConfig *pConfig;
extern bool bHudScaleFix;
extern bool r5ShowCameraTarget;
class R5ChatInfoView { BYTE reserved0[0x126]; DWORD infoColor; public: void AddInfoMessage(char *format,...); };
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

// Pointer-only access to real CGame head-movement field59, never instantiated.
#pragma pack(push,1)
struct R5GameHeadMoveView { BYTE prefix[0x59]; BOOL headMovement; };
class R5NetworkNameTagStatusView {
 BYTE prefix[0x234]; BYTE playerStatus;
public: void TogglePlayerStatus();
};
#pragma pack(pop)
void R5NetworkNameTagStatusView::TogglePlayerStatus()
{
 if (!playerStatus) {
  ((R5ChatInfoView*)pChatWindow)->AddInfoMessage("NameTag Player Status: ON");
  if(pConfig) pConfig->SetIntVariable("nonametagstatus",0);
  playerStatus=1;
 } else {
  ((R5ChatInfoView*)pChatWindow)->AddInfoMessage("NameTag Player Status: OFF");
  if(pConfig) pConfig->SetIntVariable("nonametagstatus",1);
  playerStatus=0;
 }
}
void cmdCameraTargetDebug(PCHAR szCmd) { r5ShowCameraTarget=!r5ShowCameraTarget; }
void cmdNameTagStatus(PCHAR szCmd)
{ if(pNetGame) ((R5NetworkNameTagStatusView*)pNetGame)->TogglePlayerStatus(); }
void cmdPageSize(PCHAR szCmd)
{
 int lines;
 if (strlen(szCmd) && (lines=atoi(szCmd))>=10 && lines<=20) {
  if(pChatWindow) ((R5ChatPagingView*)pChatWindow)->SetPageSize(lines);
  if(pConfig) pConfig->SetIntVariable("pagesize",lines);
 } else {
  if(pChatWindow) pChatWindow->AddDebugMessage("pagesize [10-20] (lines)");
 }
}
void cmdTimestamp(PCHAR szCmd)
{
 if(pChatWindow) {
  R5ChatPagingView *chat=(R5ChatPagingView*)pChatWindow;
  if(chat->timestamps) {
   chat->timestamps=0;chat->cacheDirty=1;
   if(pConfig) pConfig->SetIntVariable("timestamp",0);
  } else {
   chat->timestamps=1;chat->cacheDirty=1;
   if(pConfig) pConfig->SetIntVariable("timestamp",1);
  }
 }
}
void cmdAudioMsg(PCHAR szCmd)
{
 if(pConfig->GetIntVariable("audiomsgoff")==1) {
  pConfig->SetIntVariable("audiomsgoff",0);
  if(pChatWindow) ((R5ChatInfoView*)pChatWindow)->AddInfoMessage("Audio messages: On");
 } else {
  pConfig->SetIntVariable("audiomsgoff",1);
  if(pChatWindow) ((R5ChatInfoView*)pChatWindow)->AddInfoMessage("Audio messages: Off");
 }
}
void cmdLogUrls(PCHAR szCmd)
{
 if(pConfig->GetIntVariable("logurls")==1) {
  pConfig->SetIntVariable("logurls",0);
  if(pChatWindow) ((R5ChatInfoView*)pChatWindow)->AddInfoMessage("URL messages: Off");
 } else {
  pConfig->SetIntVariable("logurls",1);
  if(pChatWindow) ((R5ChatInfoView*)pChatWindow)->AddInfoMessage("URL messages: On");
 }
}
void cmdSetFrameLimit(PCHAR szCmd)
{
 if(strlen(szCmd)) {
  DWORD limit=atoi(szCmd);
  if(limit>=20 && limit<=90) {
   pGame->SetFrameLimit(limit);
   if(pConfig) pConfig->SetIntVariable("fpslimit",limit);
   ((R5ChatInfoView*)pChatWindow)->AddInfoMessage("-> Frame Limiter: %u",limit);
  } else {
   // R5 really supplies this extra vararg, although the format has no conversion.
   ((R5ChatInfoView*)pChatWindow)->AddInfoMessage("-> Frame Limiter: valid amounts are 20-90",limit);
  }
 }
}
void cmdHeadMove(PCHAR szCmd)
{
 if(pGame) {
  R5GameHeadMoveView *game=(R5GameHeadMoveView*)pGame;
  if(game->headMovement) {
   game->headMovement=0;
   if(pConfig) pConfig->SetIntVariable("disableheadmove",1);
   ((R5ChatInfoView*)pChatWindow)->AddInfoMessage("-> Head movements disabled");
  } else {
   game->headMovement=1;
   if(pConfig) pConfig->SetIntVariable("disableheadmove",0);
   ((R5ChatInfoView*)pChatWindow)->AddInfoMessage("-> Head movements enabled");
  }
 }
}

void cmdHudScaleFix(PCHAR szCmd)
{
	if(bHudScaleFix)
	{
		bHudScaleFix = false;
		pConfig->SetIntVariable("nohudscalefix", 1);
	}
	else
	{
		bHudScaleFix = true;
		pConfig->SetIntVariable("nohudscalefix", 0);
	}
}

void cmdMem(PCHAR szCmd)
{
	pChatWindow->AddDebugMessage("Memory: %u",*(DWORD *)0x8A5A80);
}
