#include "main.h"
extern CNetGame *pNetGame;
#pragma pack(push,1)
struct R5RemoteVehicleExitRequestView {
 BYTE unknown0[0x1dd]; CPlayerPed *ped;
 void ExitVehicle();
};
struct R5RemoteDistanceQueryView {
 BYTE unknown0[0x10a]; BYTE state;
 BYTE unknown10B[0xd2]; CPlayerPed *ped; CVehicle *vehicle;
 float GetDistanceFromLocalPlayer();
};
struct R5ExitRpcPlayerPoolView {
 BYTE prefix[0x1f8a]; CNetPlayer *players[MAX_PLAYERS];
 R5RemoteVehicleExitRequestView *GetRemote(PLAYERID id) {
  if(id>MAX_PLAYERS)return NULL;
  CNetPlayer *player=players[id];
  if(!player)return NULL;
  return (R5RemoteVehicleExitRequestView*)player->m_pRemotePlayer;
 }
};
struct R5ExitRpcPoolsView { void *vehiclePool; R5ExitRpcPlayerPoolView *playerPool; };
struct R5ExitRpcNetView {
 BYTE prefix[0x3de]; R5ExitRpcPoolsView *pools;
 R5ExitRpcPlayerPoolView *GetPlayerPool(){return pools->playerPool;}
};
#pragma pack(pop)
void RemoteExitVehicleTransfer(RPCParameters *rpcParams)
{
 PCHAR data=(PCHAR)rpcParams->input;
 int bitLength=rpcParams->numberOfBitsOfData;
 RakNet::BitStream stream(data,(bitLength/8)+1,false);
 PLAYERID playerId;
 int vehicleId=0;
 stream.Read(playerId);
 stream.Read((WORD&)vehicleId);
 R5RemoteVehicleExitRequestView *player=((R5ExitRpcNetView*)pNetGame)->GetPlayerPool()->GetRemote(playerId);
 if(player && ((R5RemoteDistanceQueryView*)player)->GetDistanceFromLocalPlayer()<200.0f)player->ExitVehicle();
}
