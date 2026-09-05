// R5 model/object lifetime dependency closures, partial views only.
#include "main.h"
#include "game/util.h"
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

struct RPCCustomModelDownloadWorldView;
extern RPCCustomModelDownloadWorldView *rpc_custom_model_download_manager;
class RPCEmptyCustomModelReleaseView { public: void Release(int model); };
// Original D1D0 is the whole three-byte ret4 function; no behavior is elided.
// The real empty provider is compiled separately, preserving its original call.
inline OBJECT_TYPE *RPCModelObjectPoolStart() {
 OBJECT_TYPE *object;
 // Existing base GamePool_GetObject symbolic assembly, inlined at R5 callsite.
 _asm mov eax,0xB7449C
 _asm mov edx,[eax]
 _asm mov eax,[edx]
 _asm mov object,eax
 return object;
}
bool RPCIsObjectModelUsed(int model) {
 OBJECT_TYPE *object=RPCModelObjectPoolStart();
 for(int i=0;i!=3000;i++,object++) {
  if(object && object->vtable && object->vtable!=0x863c40 && object->nModelIndex==model)return true;
 }
 return false;
}
const SCRIPT_COMMAND rpc_release_model={0x0249,"i"};
class RPCGameModelReleaseView { public: void RemoveModel(int model,BYTE force); };
void RPCGameModelReleaseView::RemoveModel(int model,BYTE force) {
 if(model>=0) {
  if(model<20000) {
   if(force) {
    if(ScriptCommand(&is_model_available,model)) {
     // Original 0.2.5 RemoveModel native call retained symbolically.
     _asm push model
     _asm mov edx,0x4089A0
     _asm call edx
     _asm pop edx
    }
   } else {
    if(ScriptCommand(&is_model_available,model))ScriptCommand(&rpc_release_model,model);
   }
  }
 } else {
  if(rpc_custom_model_download_manager && ((R5CustomModelManagerView*)rpc_custom_model_download_manager)->HasTextureModel(model)) {
   if(!RPCIsObjectModelUsed(model))((RPCEmptyCustomModelReleaseView*)rpc_custom_model_download_manager)->Release(model);
  }
 }
}
struct RPCObjectMaterialReleaseView {
 BYTE gap[0x31b]; DWORD materialsPresent[16]; BYTE gap35b[0xdf0]; IUnknown *textures[16];
 void ReleaseMaterialTextures();
};
void RPCObjectMaterialReleaseView::ReleaseMaterialTextures() {
 for(int i=0;i<16;i++) {
  if(textures[i]) { textures[i]->Release(); textures[i]=NULL; materialsPresent[i]=0; }
 }
}
