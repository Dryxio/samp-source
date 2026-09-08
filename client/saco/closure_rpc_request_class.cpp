// Source025 RequestClass/HandleClassSelectionOutcome, adapted to verified R5
// packed spawn payload and R5 unconditional local-player dereference.
#include "main.h"
#include <stddef.h>
extern CNetGame *pNetGame;
#pragma pack(push,1)
struct RPCSpawnInfoView { BYTE initial[6]; VECTOR position; BYTE remaining[28]; };
struct RPCSpawnLocalView {
 BYTE gap[0x14f]; RPCSpawnInfoView spawnInfo; BOOL hasSpawnInfo;
 void SetSpawnInfo(RPCSpawnInfoView *info);
};
struct R5ClassOutcomeLocalView {
 BYTE gap0[0x104]; CPlayerPed *ped;
 BYTE gap108[0x143-0x108]; BOOL cleared;
 BYTE gap147[0x150-0x147]; int skin;
 void HandleClassSelectionOutcome(BOOL outcome);
};
struct R5ClassRpcNetView { BYTE gap[0x3de]; NETGAME_POOLS *pools; };
#pragma pack(pop)
class R5PreviewPedModelView {
 BYTE unknown0[0x44]; DWORD gtaId; BYTE unknown48[0x25c]; PED_TYPE *ped;
public: void SetModelIndex(UINT model);
};
typedef char CheckSpawnWire46[sizeof(RPCSpawnInfoView)==46?1:-1];
typedef char CheckClassSkin[offsetof(R5ClassOutcomeLocalView,skin)==0x150?1:-1];
void R5ClassOutcomeLocalView::HandleClassSelectionOutcome(BOOL outcome) {
 if(outcome) {
  if(ped) {
   ped->ClearAllWeapons();
   ((R5PreviewPedModelView*)ped)->SetModelIndex(skin);
  }
  cleared=TRUE;
 }
}
void RequestClassRPCTransfer(RPCParameters *rpcParams) {
 PCHAR data=(PCHAR)rpcParams->input;
 int bits=rpcParams->numberOfBitsOfData;
 RakNet::BitStream stream(data,bits/8+1,false);
 BYTE outcome=0;
 RPCSpawnInfoView info;
 CPlayerPool *pool=((R5ClassRpcNetView*)pNetGame)->pools->pPlayerPool;
 R5ClassOutcomeLocalView *player=(R5ClassOutcomeLocalView*)pool->GetLocalPlayer();
 stream.Read(outcome);
 stream.Read((PCHAR)&info,sizeof(info));
 if(outcome) {
  ((RPCSpawnLocalView*)player)->SetSpawnInfo(&info);
  player->HandleClassSelectionOutcome(TRUE);
 } else player->HandleClassSelectionOutcome(FALSE);
}
