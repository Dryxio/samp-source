// Reconstructed R5 allocation storage, /Zp1. Reserved bytes have no invented semantics.
#include <windows.h>
#include <stddef.h>
struct R5TextDrawTransmit {
    BYTE flags;
    float letterWidth,letterHeight;
    DWORD letterColor;
    float lineWidth,lineHeight;
    DWORD boxColor;
    BYTE shadow,outline;
    DWORD backgroundColor;
    BYTE style,selectable;
    float x,y;
    WORD previewModel;
    float rotationX,rotationY,rotationZ,zoom;
    WORD color1,color2;
};
struct R5TextDrawConstructionData {
    float letterWidth,letterHeight;
    DWORD letterColor;
    BYTE unknown12,centered,box;
    float lineWidth,lineHeight;
    DWORD boxColor;
    BYTE proportional;
    DWORD backgroundColor;
    BYTE shadow,outline,alignLeft,alignRight;
    DWORD style;
    float x,y;
    BYTE reservedGxt[8];
    DWORD parameter1,parameter2;
    int textureSlot;
    BYTE selectable;
    WORD previewModel;
    float rotationX,rotationY,rotationZ,zoom;
    WORD color1,color2;
};
class R5TextDrawConstructionView {
public:
    char text[801];
    char expandedText[1602];
    R5TextDrawConstructionData data;
    BYTE reserved9be,drawn,keyCodes;
    RECT bounds;
    BYTE selected;
    DWORD selectedColor;
    R5TextDrawConstructionView(R5TextDrawTransmit *transmit,char *value);
    void SetText(char *value);
    void LoadTexture();
};
typedef char r5_transmit63[(sizeof(R5TextDrawTransmit)==63)?1:-1];
typedef char r5_construction_data91[(sizeof(R5TextDrawConstructionData)==91)?1:-1];
typedef char r5_textdraw_allocation9d6[(sizeof(R5TextDrawConstructionView)==0x9d6)?1:-1];
typedef char r5_textdraw_data963[(offsetof(R5TextDrawConstructionView,data)==0x963)?1:-1];
typedef char r5_textdraw_bounds9c1[(offsetof(R5TextDrawConstructionView,bounds)==0x9c1)?1:-1];

#define MAX_TEXT_DRAWS 2304
class R5TextDrawCleanupView {
    unsigned char unknown_prefix[0x987];
    DWORD style;
    unsigned char unknown_between[0x18];
    int textureSlot;
public:
    ~R5TextDrawCleanupView();
    void ReleasePreviewTexture();
};
class R5TextDrawPool {
    BOOL m_bSlotState[MAX_TEXT_DRAWS];
    R5TextDrawCleanupView* m_pTextDraw[MAX_TEXT_DRAWS];
public:
    R5TextDrawPool();
    ~R5TextDrawPool();
    void ReleasePreviewTextures();
    void Delete(WORD wText);
};

class R5TextDrawPoolConstructionView {
    BOOL slotState[2304];
    R5TextDrawConstructionView *textdraw[2304];
public:
    R5TextDrawConstructionView *New(WORD id,R5TextDrawTransmit *transmit,char *value);
};
R5TextDrawConstructionView *R5TextDrawPoolConstructionView::New(WORD id,R5TextDrawTransmit *transmit,char *value)
{
    if(textdraw[id]) ((R5TextDrawPool*)this)->Delete(id);
    R5TextDrawConstructionView *result=new R5TextDrawConstructionView(transmit,value);
    if(result) {
        textdraw[id]=result;
        slotState[id]=TRUE;
        return result;
    }
    return NULL;
}
