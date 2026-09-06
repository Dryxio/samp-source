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
void ReleaseTextdrawTexture(int);
int AllocateTextdrawTexture();
void R5TextDrawConstructionView::SetText(char *value)
{
    memset(text,0,800);
    if(strlen(value)>=800) return;
    strncpy(text,value,800);
    text[800]=0;
    keyCodes=0;
    bool inside=false;
    int tokenLength=0;
    char *cursor=text;
    bool valid=true;
    while(*cursor) {
        if(*cursor=='~') {
            if(!inside) {
                inside=true;
                if(cursor[1]=='k' || cursor[1]=='K') keyCodes=1;
            } else {
                tokenLength=0;
                inside=false;
            }
        }
        if(inside) {
            tokenLength++;
            if(tokenLength>24) valid=false;
        }
        cursor++;
    }
    if(!valid || inside) {
        strcpy(text,"Error: unmatched tilde");
        keyCodes=0;
    }
    if(keyCodes && strlen(text)>255) {
        strcpy(text,"Error: can't use k-codes in long string");
        keyCodes=0;
    }
    if(data.style==4 && data.textureSlot!=-1) {
        ReleaseTextdrawTexture(data.textureSlot);
        data.textureSlot=AllocateTextdrawTexture();
        LoadTexture();
    }
}
