// Adapted025 attachment/spawn RPCs and R5/base world helpers.
#include "main.h"
extern CNetGame *pNetGame;
extern CGame *pGame;
struct RPCWorldExtraRemoteView {
 BYTE first[4]; BOOL showNameTag; BYTE gap8[0x1b9]; BOOL customAnimation;
 BYTE gap1c5[0x18]; CPlayerPed *ped;
 CPlayerPed *GetPlayerPed() { return ped; }
 void ShowNameTag(BYTE show) { showNameTag=(show!=0); }
};
struct RPCWorldExtraNetPlayerView { BYTE gap[0x10]; RPCWorldExtraRemoteView *remote; };
struct RPCWorldExtraPlayerPoolView {
 BYTE first[4]; WORD localId; BYTE gap6[0x20]; CLocalPlayer *local;
 BOOL slots[1004]; BYTE gapfda[4016]; RPCWorldExtraNetPlayerView *players[1004];
 WORD GetLocalPlayerID() { return localId; }
 CLocalPlayer *GetLocalPlayer() { return local; }
 BOOL GetSlotState(WORD id) { if(id>=1004)return FALSE; return slots[id]; }
 RPCWorldExtraRemoteView *GetAt(WORD id) { if(id>1004)return NULL; RPCWorldExtraNetPlayerView *p=players[id]; if(p)return p->remote; return NULL; }
};
struct RPCWorldExtraNetGameView { BYTE gap[0x3de]; NETGAME_POOLS *pools; };
inline RPCWorldExtraPlayerPoolView *RPCWorldExtraPlayers() { return (RPCWorldExtraPlayerPoolView*)((RPCWorldExtraNetGameView*)pNetGame)->pools->pPlayerPool; }
struct RPCWorldExtraCameraLocalView { BYTE gap[0x314]; BOOL cameraControlled; };
struct RPCWorldExtraCameraObjectPoolView {
 int lastId; BOOL slots[1000]; CEntity *objects[1000];
 CEntity *GetAt(WORD id) { if(id>1000)return NULL; if(slots[id])return objects[id]; return NULL; }
 BOOL GetSlotState(WORD id) { if(id>1000)return FALSE; return slots[id]; }
};
inline RPCWorldExtraCameraObjectPoolView *RPCWorldExtraCameraObjects() { return (RPCWorldExtraCameraObjectPoolView*)((RPCWorldExtraNetGameView*)pNetGame)->pools->pObjectPool; }

#include "game/util.h"
struct RPCCustomModelDownloadWorldView;
extern RPCCustomModelDownloadWorldView *rpc_custom_model_download_manager;
extern bool GLOBAL_101506A4;
const SCRIPT_COMMAND rpc_attach_object_to_actor={0x069b,"iiffffff"};
struct RPCSpawnInfoView { BYTE initial[6]; VECTOR position; BYTE remaining[28]; };
struct RPCSpawnLocalView {
 BYTE gap[0x14f]; RPCSpawnInfoView spawnInfo; BOOL hasSpawnInfo;
 void SetSpawnInfo(RPCSpawnInfoView *info);
};
void RPCSpawnLocalView::SetSpawnInfo(RPCSpawnInfoView *info) {
 if(FUNC_100B4B50(&info->position)) { memcpy(&spawnInfo,info,sizeof(RPCSpawnInfoView)); hasSpawnInfo=TRUE; }
}
void CGame::SetTimeInMilliseconds(DWORD dwTimeInMs) {
 if(!field_69) { *(DWORD*)0xB7CB84=dwTimeInMs&0x3fffffff; GLOBAL_101506A4=true; }
}
struct RPCCustomModelDownloadWorldView { BYTE gap[0x217]; int currentWorld; BYTE gap21b[5]; BYTE changedA; BYTE changedB; };
struct RPCDeleteLabelPoolView {
 BYTE labels[0xe800]; BOOL slots[2048];
 BOOL Delete(WORD id);
};
// Delete identity remains existing CLabelPool::Delete at12E0; view only checks slots.
void Delete3DTextLabelTransfer(RPCParameters *rpcParams) {
 PCHAR Data=reinterpret_cast<PCHAR>(rpcParams->input);
 int bits=rpcParams->numberOfBitsOfData;
 RakNet::BitStream bsData(Data,(bits/8)+1,false);
 CLabelPool *pool=((RPCWorldExtraNetGameView*)pNetGame)->pools->pLabelPool;
 if(!pool)return;
 WORD id; bsData.Read(id);
 if(id<2048 && ((RPCDeleteLabelPoolView*)pool)->slots[id]) pool->Delete(id);
}
void SetTimeMillisecondsTransfer(RPCParameters *rpcParams) {
 PCHAR Data=reinterpret_cast<PCHAR>(rpcParams->input);
 int bits=rpcParams->numberOfBitsOfData;
 RakNet::BitStream bsData(Data,(bits/8)+1,false);
 DWORD time=0;
 bsData.Read(time);
 pGame->SetTimeInMilliseconds(time);
}
void ScrSetCustomModelWorldTransfer(RPCParameters *rpcParams) {
 PCHAR Data=reinterpret_cast<PCHAR>(rpcParams->input);
 int bits=rpcParams->numberOfBitsOfData;
 RakNet::BitStream bsData(Data,(bits/8)+1,false);
 int size=0;
 bsData.Read(size);
 RPCCustomModelDownloadWorldView *manager=rpc_custom_model_download_manager;
 if(manager && manager->currentWorld!=size) { manager->currentWorld=size; manager->changedB=0; manager->changedA=0; }
}
void ScrSetSpawnInfoTransfer(RPCParameters *rpcParams) {
 PCHAR Data=reinterpret_cast<PCHAR>(rpcParams->input);
 int bits=rpcParams->numberOfBitsOfData;
 RPCSpawnInfoView info;
 RakNet::BitStream bsData(Data,(bits/8)+1,false);
 CPlayerPool *pool=pNetGame->GetPlayerPool();
 bsData.Read((PCHAR)&info,sizeof(info));
 ((RPCSpawnLocalView*)pool->GetLocalPlayer())->SetSpawnInfo(&info);
}
void ScrAttachObjectToPlayerTransfer(RPCParameters *rpcParams) {
 PCHAR Data=reinterpret_cast<PCHAR>(rpcParams->input);
 int bits=rpcParams->numberOfBitsOfData;
 RakNet::BitStream bsData(Data,(bits/8)+1,false);
 WORD objectId; PLAYERID playerId;
 float OffsetX,OffsetY,OffsetZ,rX,rY,rZ;
 bsData.Read(objectId); bsData.Read(playerId);
 bsData.Read(OffsetX); bsData.Read(OffsetY); bsData.Read(OffsetZ);
 bsData.Read(rX); bsData.Read(rY); bsData.Read(rZ);
 CEntity *object=RPCWorldExtraCameraObjects()->GetAt(objectId);
 if(!object)return;
 if(playerId==RPCWorldExtraPlayers()->GetLocalPlayerID()) {
  CLocalPlayer *player=RPCWorldExtraPlayers()->GetLocalPlayer();
  ScriptCommand(&rpc_attach_object_to_actor,object->m_dwGTAId,player->GetPlayerPed()->m_dwGTAId,OffsetX,OffsetY,OffsetZ,rX,rY,rZ);
 } else {
  RPCWorldExtraRemoteView *remote=RPCWorldExtraPlayers()->GetAt(playerId);
  if(!remote)return;
  ScriptCommand(&rpc_attach_object_to_actor,object->m_dwGTAId,remote->GetPlayerPed()->m_dwGTAId,OffsetX,OffsetY,OffsetZ,rX,rY,rZ);
 }
}
