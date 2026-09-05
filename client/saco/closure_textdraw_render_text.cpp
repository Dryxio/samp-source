// Adapted from local0.2.5 client/game/textdraw.cpp Draw(), with observed R5 additions.
#include <windows.h>
#include <string.h>
#include "game/font.h"
// Pointer-only R5 rendering view. Never allocate; unknown bytes receive no coverage.
#include <windows.h>
struct R5TextDrawRenderData {
    float fLetterWidth,fLetterHeight;
    DWORD dwLetterColor;
    BYTE byteUnk12,byteCentered,byteBox;
    float fLineWidth,fLineHeight;
    DWORD dwBoxColor;
    BYTE byteProportional;
    DWORD dwBackgroundColor;
    BYTE byteShadow,byteOutline,byteAlignLeft,byteAlignRight;
    DWORD dwStyle;
    float fX,fY;
    unsigned char unknownGxt[8];
    DWORD unknownParam1,unknownParam2;
    int textureSlot;
    unsigned char unknownPreview[23];
};
class R5TextDrawRenderView {
public:
    char m_szText[801];
    char m_szString[1602];
    R5TextDrawRenderData m_TextDrawData;
    BYTE unknown9be;
    BYTE drawn;
    BYTE keyCodes;
    RECT bounds;
    BYTE selected;
    DWORD selectedColor;
    void DrawText();
    void DrawSprite();
    void Draw();
};

#include <stddef.h>
typedef char r5_render_data_offset[(offsetof(R5TextDrawRenderView,m_TextDrawData)==0x963)?1:-1];
typedef char r5_render_slot_offset[(offsetof(R5TextDrawRenderView,m_TextDrawData)+offsetof(R5TextDrawRenderData,textureSlot)==0x9A3)?1:-1];
typedef char r5_render_bounds_offset[(offsetof(R5TextDrawRenderView,bounds)==0x9C1)?1:-1];
typedef char r5_render_selected_offset[(offsetof(R5TextDrawRenderView,selected)==0x9D1)?1:-1];

void R5TextDrawRenderView::DrawText()
{
    if(!m_szText || !strlen(m_szText)) return;
    strcpy(m_szString,m_szText);
    int iScreenWidth,iScreenHeight;
    float fVertHudScale,fHorizHudScale;
    iScreenWidth=*(int*)0xC17044;
    iScreenHeight=*(int*)0xC17048;
    fVertHudScale=*(float*)0x859524;
    fHorizHudScale=*(float*)0x859520;
    float fScaleY=(float)iScreenHeight*m_TextDrawData.fLetterHeight*fVertHudScale*0.5f;
    float fScaleX=(float)iScreenWidth*m_TextDrawData.fLetterWidth*fHorizHudScale;
    Font_SetScale(fScaleX,fScaleY);
    if(selected) Font_SetColor(selectedColor);
    else Font_SetColor(m_TextDrawData.dwLetterColor);
    Font_Unk12(0);
    if(m_TextDrawData.byteAlignRight) Font_SetJustify(2);
    else if(m_TextDrawData.byteCentered) Font_SetJustify(0);
    else Font_SetJustify(1);
    float fLineWidth=iScreenWidth*m_TextDrawData.fLineWidth*fHorizHudScale;
    Font_SetLineWidth(fLineWidth);
    float fLineHeight=iScreenWidth*m_TextDrawData.fLineHeight*fHorizHudScale;
    Font_SetLineHeight(fLineHeight);
    Font_UseBox(m_TextDrawData.byteBox,0);
    Font_UseBoxColor(m_TextDrawData.dwBoxColor);
    Font_SetProportional(m_TextDrawData.byteProportional);
    Font_SetDropColor(m_TextDrawData.dwBackgroundColor);
    if(m_TextDrawData.byteOutline) Font_SetOutline(m_TextDrawData.byteOutline);
    else Font_SetShadow(m_TextDrawData.byteShadow);
    Font_SetFontStyle(m_TextDrawData.dwStyle);
    Font_UnkConv(m_szString,-1,-1,-1,-1,-1,-1,m_szString);
    if(keyCodes) Font_UnkConv2(m_szString);
    float fPS2Height=448.0f;
    float fPS2Width=640.0f;
    float fScriptY=m_TextDrawData.fY;
    float fScriptX=m_TextDrawData.fX;
    float fUseX,fUseY;
    fUseY=iScreenHeight-((fPS2Height-fScriptY)*(iScreenHeight*fVertHudScale));
    fUseX=iScreenWidth-((fPS2Width-fScriptX)*(iScreenWidth*fHorizHudScale));
    Font_PrintString(fUseX,fUseY,m_szString);
    Font_SetOutline(0);
    // Preserve R5 rectangle arithmetic, including its centered branch dimensions.
    if(m_TextDrawData.byteAlignRight) {
        bounds.left=(int)(fUseX-(fLineWidth-fUseX));
        bounds.right=(int)fUseX;
        bounds.top=(int)fUseY;
        bounds.bottom=(int)(fUseY+fLineHeight);
    } else if(m_TextDrawData.byteCentered) {
        bounds.left=(int)(fUseX-fLineHeight*0.5f);
        bounds.right=bounds.left+(int)fLineHeight;
        bounds.top=(int)fUseY;
        bounds.bottom=(int)(fUseY+fLineWidth);
    } else {
        bounds.left=(int)fUseX;
        bounds.right=(int)(fUseX+(fLineWidth-fUseX));
        bounds.top=(int)fUseY;
        bounds.bottom=(int)(fUseY+fLineHeight);
    }
    drawn=1;
}
