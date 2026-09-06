#include "main.h"
extern CNetGame *pNetGame;
#pragma pack(push,1)
struct R5RemoteVehicleEntryRequestView {
 BYTE unknown0[0x1dd]; CPlayerPed *ped;
 void EnterVehicle(VEHICLEID id,BOOL passenger);
};
struct R5RemoteDistanceQueryView {
 BYTE unknown0[0x10a]; BYTE state;
 BYTE unknown10B[0xd2]; CPlayerPed *ped; CVehicle *vehicle;
 float GetDistanceFromLocalPlayer();
};
struct R5EnterRpcPlayerPoolView {
 BYTE prefix[0x1f8a]; CNetPlayer *players[MAX_PLAYERS];
 R5RemoteVehicleEntryRequestView *GetRemote(PLAYERID id) {
  if(id>MAX_PLAYERS)return NULL;
  CNetPlayer *player=players[id];
  if(!player)return NULL;
  return (R5RemoteVehicleEntryRequestView*)player->m_pRemotePlayer;
 }
};
struct R5EnterRpcPoolsView { void *vehiclePool; R5EnterRpcPlayerPoolView *playerPool; };
struct R5EnterRpcNetView {
 BYTE prefix[0x3de]; R5EnterRpcPoolsView *pools;
 R5EnterRpcPlayerPoolView *GetPlayerPool(){return pools->playerPool;}
};
#pragma pack(pop)
void RemoteEnterVehicleTransfer(RPCParameters *rpcParams)
{
 PCHAR data=(PCHAR)rpcParams->input;
 int bitLength=rpcParams->numberOfBitsOfData;
 RakNet::BitStream stream(data,(bitLength/8)+1,false);
 PLAYERID playerId;
 int vehicleId=0;
 BYTE passengerFlag=0;
 BOOL passenger=FALSE;
 stream.Read(playerId);
 stream.Read((WORD&)vehicleId);
 stream.Read(passengerFlag);
 if(passengerFlag)passenger=TRUE;
 R5RemoteVehicleEntryRequestView *player=((R5EnterRpcNetView*)pNetGame)->GetPlayerPool()->GetRemote(playerId);
 if(player && ((R5RemoteDistanceQueryView*)player)->GetDistanceFromLocalPlayer()<200.0f)player->EnterVehicle(vehicleId,passenger);
}
