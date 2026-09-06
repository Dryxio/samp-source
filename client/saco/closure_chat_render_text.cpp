// R5 chat renderer67470/851. Partial pointer view, never allocated.
#include "main.h"
extern void RemoveColorEmbedsFromString(char *text);
#pragma pack(push,1)
class R5ChatTextRenderView
{
public:
 BYTE prefix[8];
 int displayMode;
 BYTE gapAfterMode[0x63A2-12];
 CFontRender *font;
 ID3DXSprite *sprite;
 void RenderText(char *text,RECT rect,DWORD color);
};
#pragma pack(pop)
void R5ChatTextRenderView::RenderText(char *text,RECT rect,DWORD color)
{
 ID3DXFont *shadowFont=font->field_8;
 ID3DXFontHook *textFont=font->field_0;
 int textLength=strlen(text);
 if(displayMode==2)
 {
  char stripped[512]={0};
  strncpy(stripped,text,511);
  RemoveColorEmbedsFromString(stripped);
  int shadowLength=strlen(stripped);
  const DWORD NativeScreenWidth=0xC17044;
  if(*(int*)NativeScreenWidth>1280)
  {
   rect.top-=2; rect.bottom-=2;
   shadowFont->DrawTextA(sprite,stripped,shadowLength,&rect,DT_NOCLIP|DT_SINGLELINE,0xFF000000);
   rect.top+=1; rect.bottom+=1;
   rect.left-=1; rect.right-=1;
   shadowFont->DrawTextA(sprite,stripped,shadowLength,&rect,DT_NOCLIP|DT_SINGLELINE,0xFF000000);
   rect.left+=1; rect.right+=1;
   rect.left+=1; rect.right+=1;
   shadowFont->DrawTextA(sprite,stripped,shadowLength,&rect,DT_NOCLIP|DT_SINGLELINE,0xFF000000);
   rect.left-=1; rect.right-=1;
   rect.top+=1; rect.bottom+=1;
   rect.top+=2; rect.bottom+=2;
   shadowFont->DrawTextA(sprite,stripped,shadowLength,&rect,DT_NOCLIP|DT_SINGLELINE,0xFF000000);
   rect.top-=1; rect.bottom-=1;
   rect.left-=1; rect.right-=1;
   shadowFont->DrawTextA(sprite,stripped,shadowLength,&rect,DT_NOCLIP|DT_SINGLELINE,0xFF000000);
   rect.left+=1; rect.right+=1;
   rect.left+=1; rect.right+=1;
   shadowFont->DrawTextA(sprite,stripped,shadowLength,&rect,DT_NOCLIP|DT_SINGLELINE,0xFF000000);
   rect.left-=1; rect.right-=1;
   rect.top-=1; rect.bottom-=1;
   rect.left-=2; rect.right-=2;
   shadowFont->DrawTextA(sprite,stripped,shadowLength,&rect,DT_NOCLIP|DT_SINGLELINE,0xFF000000);
   rect.left+=2; rect.right+=2;
   rect.left+=2; rect.right+=2;
   shadowFont->DrawTextA(sprite,stripped,shadowLength,&rect,DT_NOCLIP|DT_SINGLELINE,0xFF000000);
   rect.left-=2; rect.right-=2;
  }
  else
  {
   rect.top-=1;
   shadowFont->DrawTextA(sprite,stripped,shadowLength,&rect,DT_NOCLIP|DT_SINGLELINE,0xFF000000);
   rect.top+=2;
   shadowFont->DrawTextA(sprite,stripped,shadowLength,&rect,DT_NOCLIP|DT_SINGLELINE,0xFF000000);
   rect.top-=1; rect.left-=1;
   shadowFont->DrawTextA(sprite,stripped,shadowLength,&rect,DT_NOCLIP|DT_SINGLELINE,0xFF000000);
   rect.left+=2;
   shadowFont->DrawTextA(sprite,stripped,shadowLength,&rect,DT_NOCLIP|DT_SINGLELINE,0xFF000000);
   rect.left-=1;
  }
 }
 textFont->DrawTextA(sprite,text,textLength,&rect,DT_NOCLIP|DT_SINGLELINE,color|0xFF000000);
}
