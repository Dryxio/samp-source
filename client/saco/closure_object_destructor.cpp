// R5 destructor-only derived view. Never instantiate, allocate, or use sizeof
// as a complete CObject size: gaps denote unknown members, not absent members.
#include "main.h"
#include "game/util.h"
#include "game/textures.h"
#include <stdlib.h>
extern CGame *pGame;
extern BOOL bQuitGame;
extern unsigned short __stdcall GetModelReferenceCount(int model);
class RPCGameModelReleaseView { public: void RemoveModel(int model,BYTE force); };
struct RPCObjectMaterialReleaseView {
 BYTE gap[0x31b]; DWORD materialsPresent[16]; BYTE gap35b[0xdf0]; IUnknown *textures[16];
 void ReleaseMaterialTextures();
};
class R5ObjectDestructionView;
extern DWORD r5_shared_object_material_texture;
extern R5ObjectDestructionView *r5_rendering_material_object;
extern DWORD r5_object_material_render_active;
const SCRIPT_COMMAND r5_destroy_object={0x0108,"i"};
#pragma pack(push,1)
class R5ObjectDestructionView : public CEntity {
 BYTE unknown48[6];
 int modelId;
 BYTE unknown52[0x1c5];
 DWORD materialTextures[16]; //217
 BYTE unknown257[0xe74];
 char *materialTextBuffers[16]; //10CB
 DWORD materialPointers[16]; //110B, pointer-sized slot; pointee not accessed
 IUnknown *materialD3DTextures[16]; //114B
 DWORD hasMaterials; //118B
public:
 virtual ~R5ObjectDestructionView();
 virtual void Add();
 virtual void Remove();
};
#pragma pack(pop)
void R5ObjectDestructionView::Add() {
 if(m_pEntity && m_pEntity->vtable!=0x863c40)WorldAddEntity((DWORD*)m_pEntity);
}
void R5ObjectDestructionView::Remove() {
 if(m_pEntity && m_pEntity->vtable!=0x863c40)WorldRemoveEntity((DWORD*)m_pEntity);
}
R5ObjectDestructionView::~R5ObjectDestructionView() {
 m_pEntity=GamePool_Object_GetAt(m_dwGTAId);
 if(m_pEntity && m_pEntity->vtable!=0x863c40 && !bQuitGame) {
  if(GetRwObject())DeleteRwObject();
  ScriptCommand(&r5_destroy_object,m_dwGTAId);
  if(modelId<0 || !GetModelReferenceCount(modelId))
   ((RPCGameModelReleaseView*)pGame)->RemoveModel(modelId,0);
  if(hasMaterials) {
   ((RPCObjectMaterialReleaseView*)this)->ReleaseMaterialTextures();
   for(int i=0;i<16;i++) {
    if(materialTextures[i] && materialTextures[i]!=r5_shared_object_material_texture) {
     DestroyTexture(materialTextures[i]);
     materialTextures[i]=0;
     if(materialPointers[i])materialPointers[i]=0;
    }
    if(materialTextBuffers[i]) {
     free(materialTextBuffers[i]);
     materialTextBuffers[i]=0;
    }
   }
   if(r5_rendering_material_object==this) {
    r5_rendering_material_object=0;
    r5_object_material_render_active=0;
   }
  }
 }
}
