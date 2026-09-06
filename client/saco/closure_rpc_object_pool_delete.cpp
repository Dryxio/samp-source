#include "main.h"
extern CGame *pGame;
extern CNetGame *pNetGame;
struct RPCDeleteObjectPoolView {
 int lastId; BOOL slots[1000]; CEntity *objects[1000];
 CEntity *GetAt(WORD id) { if(id>1000)return NULL; if(slots[id])return objects[id]; return NULL; }
 void UpdateLargestID();
 BOOL Delete(WORD id);
};
struct RPCDeleteObjectPoolsView { CVehiclePool *vehicles; CPlayerPool *players; void *pickups; RPCDeleteObjectPoolView *objects; };
struct RPCDeleteObjectNetView { BYTE gap[0x3DE]; RPCDeleteObjectPoolsView *pools; RPCDeleteObjectPoolView *GetObjectPool() { return pools->objects; } CPlayerPool *GetPlayerPool() { return pools->players; } };
struct RPCDeleteObjectLocalView { BYTE gap[0x2DA]; CEntity *surfObject; DWORD gap2DE; DWORD surfMode; };
void RPCDeleteObjectPoolView::UpdateLargestID()
{
 int last=0;
 for(int i=0;i<1000;i++)if(slots[i])last=i;
 lastId=last;
}
BOOL RPCDeleteObjectPoolView::Delete(WORD id)
{
 if(id>1000 || !slots[id] || !objects[id])return FALSE;
 CCamera *camera=pGame->GetCamera();
 if(camera->field_0==objects[id])camera->FUNC_1009D660(NULL);
 slots[id]=FALSE;
 delete objects[id];
 objects[id]=NULL;
 UpdateLargestID();
 return TRUE;
}
void ScrDestroyObjectTransfer(RPCParameters *rpcParams)
{
 PCHAR Data=reinterpret_cast<PCHAR>(rpcParams->input);
 int bits=rpcParams->numberOfBitsOfData;
 WORD objectId;
 RakNet::BitStream stream(Data,(bits/8)+1,false);
 stream.Read(objectId);
 RPCDeleteObjectPoolView *pool=((RPCDeleteObjectNetView*)pNetGame)->GetObjectPool();
 RPCDeleteObjectLocalView *local=(RPCDeleteObjectLocalView*)((RPCDeleteObjectNetView*)pNetGame)->GetPlayerPool()->GetLocalPlayer();
 CEntity *object=pool->GetAt(objectId);
 if(object) {
  if(local && local->surfObject==object) {
   local->surfObject=NULL;
   local->surfMode=0;
  }
  pool->Delete(objectId);
 }
}
