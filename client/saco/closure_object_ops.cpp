// Partial R5 object views; no complete CObject, ctor or virtual table claim.
#include "main.h"
#include "game/util.h"
#include <math.h>
extern CNetGame *pNetGame;
extern BYTE *__stdcall GetModelInfo(int model);
struct RPCObjectOpsView {
 BYTE gap0[0x40]; ENTITY_TYPE *entity; DWORD gtaId; BYTE isAdded;
 BYTE gap49[0x64]; VECTOR rotation;
 BYTE gapb9[0x52]; VECTOR targetPosition;
 BYTE gap117[0x98]; BYTE moving;
 void Add(); void Remove(); BOOL IsAdded();
 float DistanceRemaining(MATRIX4X4 *matrix);
 void StopMoving(); void InstantRotate(VECTOR *angles);
};
void RPCObjectOpsView::Add() {
 if(entity && entity->vtable!=0x863c40)WorldAddEntity((DWORD*)entity);
}
void RPCObjectOpsView::Remove() {
 if(entity && entity->vtable!=0x863c40)WorldRemoveEntity((DWORD*)entity);
}
BOOL RPCObjectOpsView::IsAdded() {
 if(entity && entity->vtable!=0x863c40 && entity->pdwRenderWare && isAdded)return TRUE;
 return FALSE;
}
float RPCObjectOpsView::DistanceRemaining(MATRIX4X4 *matPos) {
 float fSX,fSY,fSZ;
 fSX=(matPos->pos.X-targetPosition.X)*(matPos->pos.X-targetPosition.X);
 fSY=(matPos->pos.Y-targetPosition.Y)*(matPos->pos.Y-targetPosition.Y);
 fSZ=(matPos->pos.Z-targetPosition.Z)*(matPos->pos.Z-targetPosition.Z);
 return (float)sqrt(fSX+fSY+fSZ);
}
void RPCObjectOpsView::StopMoving() {
 VECTOR zero;
 zero.X=0.0f; zero.Y=0.0f; zero.Z=0.0f;
 ((CEntity*)this)->SetMoveSpeedVector(zero);
 ((CEntity*)this)->SetTurnSpeedVector(zero);
 moving &= ~1;
}
const SCRIPT_COMMAND rpc_set_object_rotation={0x0453,"ifff"};
void RPCObjectOpsView::InstantRotate(VECTOR *angles) {
 if(!entity)return;
 if(!GamePool_Object_GetAt(gtaId))return;
 ScriptCommand(&rpc_set_object_rotation,gtaId,angles->X,angles->Y,angles->Z);
 rotation.X=angles->X; rotation.Y=angles->Y; rotation.Z=angles->Z;
}
// 0.2.5 symbolic assembly transferred; model lookup uses accepted R5 GetModelInfo.
// No copied instruction bytes or emit blocks.
unsigned short __stdcall GetModelReferenceCount(int model) {
 BYTE *pModelInfoStart=GetModelInfo(model);
 unsigned short usRefs=0;
 _asm mov edx,pModelInfoStart
 _asm mov bx,word ptr[edx+8]
 _asm mov usRefs,bx
 return usRefs;
}
struct RPCObjectOpsPoolView {
 int lastId; BOOL slots[1000]; RPCObjectOpsView *objects[1000];
 RPCObjectOpsView *GetAt(WORD id) { if(id>1000)return NULL; if(slots[id])return objects[id]; return NULL; }
};
struct RPCObjectOpsNetGameView { BYTE gap[0x3de]; NETGAME_POOLS *pools; };
inline RPCObjectOpsPoolView *RPCObjectOpsPool() { return (RPCObjectOpsPoolView*)((RPCObjectOpsNetGameView*)pNetGame)->pools->pObjectPool; }
void ScrSetObjectRotationTransfer(RPCParameters *rpcParams) {
 PCHAR Data=reinterpret_cast<PCHAR>(rpcParams->input);
 int bits=rpcParams->numberOfBitsOfData;
 WORD id;
 VECTOR angles;
 RakNet::BitStream bsData(Data,(bits/8)+1,false);
 bsData.Read(id); bsData.Read(angles.X); bsData.Read(angles.Y); bsData.Read(angles.Z);
 RPCObjectOpsPoolView *pool=RPCObjectOpsPool();
 if(pool) {
  RPCObjectOpsView *object=pool->GetAt(id);
  if(object)object->InstantRotate(&angles);
 }
}
// R5 payload differs from0.2.5: only object WORD id is read; no coordinates.
void ScrStopObjectTransfer(RPCParameters *rpcParams) {
 PCHAR Data=reinterpret_cast<PCHAR>(rpcParams->input);
 int bits=rpcParams->numberOfBitsOfData;
 RakNet::BitStream bsData(Data,(bits/8)+1,false);
 WORD id; bsData.Read(id);
 RPCObjectOpsView *object=RPCObjectOpsPool()->GetAt(id);
 if(object)object->StopMoving();
}
