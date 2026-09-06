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
#include <stdio.h>
#include <stdlib.h>
// Partial R5 pointer views; never allocate or index whole objects.
#include <windows.h>
struct R5CustomModelEntryView {
    unsigned char unknown0[7];
    BYTE type;
    unsigned char unknown8[8];
    int modelId;
    unsigned char unknown14[0x36];
    DWORD textureId;
    unsigned char unknown4e[10];
    bool ready;
};
class R5CustomModelManagerView {
    R5CustomModelEntryView **items;
    unsigned int count;
    R5CustomModelEntryView *GetAt(unsigned int index) { return index<count ? items[index] : 0; }
    R5CustomModelEntryView *GetAtChecked(unsigned int index) { if(index>=count) return 0; return items[index]; }
public:
    bool HasTextureModel(int id);
    bool IsModelReady(int id);
    DWORD GetTextureId(int id);
};
struct R5CustomModelLoadEntryView {
    BYTE present;
    BYTE downloaded;
    unsigned char unknown2[5];
    BYTE type;
    unsigned char unknown8[4];
    int baseModel;
    int modelId;
    unsigned char unknown14[0x32];
    DWORD dffChecksum;
    DWORD txdChecksum;
    unsigned char unknown4e[10];
    bool ready;
    bool retry;
};
class R5CustomModelLoadView {
    R5CustomModelLoadEntryView **items;
    unsigned int count;
    unsigned char unknown8;
    R5CustomModelLoadEntryView *GetAt(unsigned int index) { return index<count ? items[index] : 0; }
    char primaryPath[261];
    char secondaryPath[261];
public:
    BOOL LoadModelFiles(int baseModel,int model,DWORD dffChecksum,DWORD txdChecksum);
    BOOL EnsureModelReady(int model);
};

struct RPCCustomModelDownloadWorldView;
extern RPCCustomModelDownloadWorldView *rpc_custom_model_download_manager;
extern DWORD r5TextdrawTextures[200];
BOOL EnsureTextdrawDictionary(char *,char *,bool);
DWORD ReadTextdrawDictionaryTexture(char *,char *);
void R5TextDrawConstructionView::LoadTexture()
{
    __declspec(align(8)) char dictionary[65];
    __declspec(align(8)) char texture[65];
    __declspec(align(8)) char dictionaryName[65];
    memset(dictionary,0,sizeof(dictionary));
    memset(texture,0,sizeof(texture));
    memset(dictionaryName,0,sizeof(dictionaryName));
    char *colon=strchr(text,':');
    if(!colon || strlen(text)>=64 || strchr(text,'\\') || strchr(text,'/')) return;
    strncpy(dictionary,text,colon-text);
    strcpy(texture,colon+1);
    if(data.textureSlot==-1) return;
    if(!strcmp(dictionary,"hud")) {
        strcpy(dictionaryName,"hud");
    } else if(!strcmp(dictionary,"samaps")) {
        strcpy(dictionaryName,"samaps");
        if(!EnsureTextdrawDictionary(dictionaryName,"samaps",true)) return;
    } else if(!strcmp(dictionary,"vehicleprev")) {
        strcpy(dictionaryName,"vehicleprev");
        if(!EnsureTextdrawDictionary(dictionaryName,"vehicles128",true)) return;
    } else if(rpc_custom_model_download_manager && strlen(dictionary)<10 && !strncmp(dictionary,"mdl",3)) {
        int model=atoi(dictionary+3);
        if(!((R5CustomModelManagerView*)rpc_custom_model_download_manager)->HasTextureModel(model)) return;
        if(!((R5CustomModelManagerView*)rpc_custom_model_download_manager)->IsModelReady(model)) {
            if(!((R5CustomModelLoadView*)rpc_custom_model_download_manager)->EnsureModelReady(model)) return;
        }
        DWORD textureId=((R5CustomModelManagerView*)rpc_custom_model_download_manager)->GetTextureId(model);
        if(!textureId) return;
        sprintf(dictionaryName,"%X",textureId);
    } else {
        strncpy(dictionaryName,dictionary,10);
        if(!EnsureTextdrawDictionary(dictionaryName,dictionary,false)) return;
    }
    r5TextdrawTextures[data.textureSlot]=ReadTextdrawDictionaryTexture(dictionaryName,texture);
}
