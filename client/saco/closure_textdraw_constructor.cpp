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

#include <string.h>
int AllocateTextdrawTexture();
// Standard assignments follow local0.2.5 constructor; packed flags and preview
// tail are R5 additions, observed in complete B36E0/402.
R5TextDrawConstructionView::R5TextDrawConstructionView(R5TextDrawTransmit *transmit,char *value)
{
    memset(&data,0,sizeof(data));
    data.letterWidth=transmit->letterWidth;
    data.letterHeight=transmit->letterHeight;
    data.letterColor=transmit->letterColor;
    data.unknown12=0;
    data.centered=(transmit->flags>>3)&1;
    data.box=transmit->flags&1;
    data.lineWidth=transmit->lineWidth;
    data.lineHeight=transmit->lineHeight;
    data.boxColor=transmit->boxColor;
    data.proportional=(transmit->flags>>4)&1;
    data.backgroundColor=transmit->backgroundColor;
    data.shadow=transmit->shadow;
    data.outline=transmit->outline;
    data.alignLeft=(transmit->flags>>1)&1;
    data.alignRight=(transmit->flags>>2)&1;
    data.style=transmit->style;
    data.x=transmit->x;
    data.y=transmit->y;
    data.parameter1=0xFFFFFFFF;
    data.parameter2=0xFFFFFFFF;
    data.selectable=transmit->selectable;
    data.previewModel=transmit->previewModel;
    data.rotationX=transmit->rotationX;
    data.rotationY=transmit->rotationY;
    data.rotationZ=transmit->rotationZ;
    data.zoom=transmit->zoom;
    data.color1=transmit->color1;
    data.color2=transmit->color2;
    keyCodes=0;
    data.textureSlot=-1;
    SetText(value);
    if(data.style==4) {
        data.textureSlot=AllocateTextdrawTexture();
        LoadTexture();
    }
    drawn=0;
    bounds.left=0;
    bounds.right=0;
    bounds.top=0;
    bounds.bottom=0;
    selected=0;
    selectedColor=0;
}
