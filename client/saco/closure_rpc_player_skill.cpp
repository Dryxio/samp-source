// R5 RPC34, reconstructed from the original complete handler and accepted providers.
#include "main.h"
extern CNetGame *pNetGame;
extern CGame *pGame;
#pragma pack(push,1)
class R5LabelRemotePlayerView {
public:
 BYTE prefix[0x10A];
 BYTE state;
 BYTE gap10B[0x1DD-0x10B];
 CPlayerPed *ped;
 BOOL IsActive();
};
struct R5SkillSlotView {
 BYTE prefix[0x2A];
 BOOL slots[MAX_PLAYERS];
 BOOL GetSlotState(PLAYERID id) { if(id>=MAX_PLAYERS)return FALSE;return slots[id]; }
};
struct R5SkillNetView { BYTE prefix[0x3DE]; NETGAME_POOLS *pools; };
#pragma pack(pop)
void SetPlayerSkillLevelRPCTransfer(RPCParameters *rpcParams) {
 PCHAR data=(PCHAR)rpcParams->input;
 int bits=rpcParams->numberOfBitsOfData;
 RakNet::BitStream stream(data,bits/8+1,false);
 PLAYERID id;
 int skill;
 WORD level;
 stream.Read(id);stream.Read(skill);stream.Read(level);
 CPlayerPool *pool=((R5SkillNetView*)pNetGame)->pools->pPlayerPool;
 if(pool) {
  if(id==pool->GetLocalPlayerID()) {
   CPlayerPed *ped=pGame->FindPlayerPed();
   if(ped)ped->SetWeaponSkillLevel(skill,level);
  } else if(((R5SkillSlotView*)pool)->GetSlotState(id)) {
   R5LabelRemotePlayerView *remote=(R5LabelRemotePlayerView*)pool->GetAt(id);
   if(remote && remote->IsActive()) {
    CPlayerPed *ped=remote->ped;
    if(ped)ped->SetWeaponSkillLevel(skill,level);
   }
  }
 }
}
