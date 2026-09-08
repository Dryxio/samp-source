#include "main.h"
#include "game/util.h"
#pragma pack(push,1)
class R5DeathPedView {
 BYTE prefix[0x44]; DWORD gtaId;
 BYTE gap48[0x25c]; PED_TYPE *ped;
 BYTE gap2a8[8]; BYTE playerNumber;
public:
 void ExtinguishFire();
 void SetDead();
};
struct R5NativePedFireView { BYTE prefix[0x730]; DWORD fire; };
struct R5NativeVehicleFireView { BYTE prefix[0x490]; DWORD fire; };
#pragma pack(pop)
#pragma pack(push,1)
struct R5RemoteDeathView {
 BYTE prefix[0x10a]; BYTE state;
 BYTE gap10b[0xd2]; CPlayerPed *ped;
 void HandleDeath();
};
#pragma pack(pop)
extern CNetGame *pNetGame;
#pragma pack(push,1)
struct R5DeathRPCPlayerPoolView {
 BYTE prefix[0x1f8a]; CNetPlayer *players[MAX_PLAYERS];
 R5RemoteDeathView *GetRemote(PLAYERID id) {
  if(id>MAX_PLAYERS)return NULL;
  CNetPlayer *player=players[id];
  if(!player)return NULL;
  return (R5RemoteDeathView*)player->m_pRemotePlayer;
 }
};
struct R5DeathRPCPoolsView { void *vehiclePool; R5DeathRPCPlayerPoolView *playerPool; };
struct R5DeathRPCNetView { BYTE prefix[0x3de]; R5DeathRPCPoolsView *pools; };
#pragma pack(pop)
void WorldPlayerDeathRPCTransfer(RPCParameters *rpcParams) {
 PCHAR data=(PCHAR)rpcParams->input;
 int bits=rpcParams->numberOfBitsOfData;
 RakNet::BitStream stream(data,bits/8+1,false);
 R5DeathRPCPlayerPoolView *pool=((R5DeathRPCNetView*)pNetGame)->pools->playerPool;
 PLAYERID id;stream.Read(id);
 if(pool) {R5RemoteDeathView *player=pool->GetRemote(id);if(player)player->HandleDeath();}
}
