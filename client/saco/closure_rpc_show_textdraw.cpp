// 0.2.5 ShowRPC skeleton with observed R5 length/error handling.
#include "main.h"
extern CNetGame *pNetGame;
extern CChatWindow *pChatWindow;
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
// Pointer-only original chat info-color field; no allocation/class size claim.
class R5ChatInfoView {
    BYTE reserved0[0x126];
    DWORD infoColor;
public:
    void AddInfoMessage(char *format,...);
};
class R5TextDrawPoolConstructionView {
    BOOL slotState[2304];
    R5TextDrawConstructionView *textdraw[2304];
public:
    R5TextDrawConstructionView *New(WORD id,R5TextDrawTransmit *transmit,char *value);
};

struct R5TextDrawShowPoolsView { BYTE reserved[0x1c]; R5TextDrawPoolConstructionView *textdraw; };
struct R5TextDrawShowNetView { BYTE reserved[0x3de]; R5TextDrawShowPoolsView *pools; };
void ScrShowTextDrawTransfer(RPCParameters *rpcParams)
{
    PCHAR Data=reinterpret_cast<PCHAR>(rpcParams->input);
    int bits=rpcParams->numberOfBitsOfData;
    PlayerID sender=rpcParams->sender;
    RakNet::BitStream stream(Data,(bits/8)+1,false);
    unsigned int length=0;
    R5TextDrawPoolConstructionView *pool=((R5TextDrawShowNetView*)pNetGame)->pools->textdraw;
    if(pool) {
        WORD id;
        R5TextDrawTransmit transmit;
        char text[800];
        stream.Read(id);
        stream.Read((char*)&transmit,sizeof(transmit));
        stream.Read(*(WORD*)&length);
        if((WORD)length>=800) {
            if(pChatWindow) ((R5ChatInfoView*)pChatWindow)->AddInfoMessage("Warning: ignoring large TextDraw size=%u",(WORD)length);
        } else {
            stream.Read(text,(WORD)length);
            text[(WORD)length]=0;
            pool->New(id,&transmit,text);
        }
    }
}
