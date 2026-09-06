#include "main.h"
#include <stddef.h>
#pragma pack(push,1)
// Complete 24-byte passenger synchronization record, with packed flag bytes
// represented as bytes because the R5 entry path extracts only the low six bits.
struct R5RemotePassengerSync {
 VEHICLEID vehicleId;
 BYTE seatAndFlags;
 BYTE weaponAndFlags;
 BYTE health;
 BYTE armour;
 WORD leftRightAnalog;
 WORD upDownAnalog;
 WORD keys;
 VECTOR position;
};
struct R5RemotePassengerUpdateView {
 BYTE unknown0[0xc];
 BYTE afk;
 BYTE unknownD[0xa0];
 R5RemotePassengerSync passengerSync;
 BYTE unknownC5[0x45];
 BYTE state;
 BYTE seat;
 BYTE unknown10C[0xa0];
 float reportedArmour;
 float reportedHealth;
 BYTE unknown1B4[4];
 BYTE updateType;
 DWORD lastReceivedTick;
 BYTE unknown1BD[0x20];
 CPlayerPed *ped;
 CVehicle *vehicle;
 BYTE unknown1E5[2];
 VEHICLEID vehicleId;
 void StorePassengerSync(R5RemotePassengerSync *sync);
};
#pragma pack(pop)

#pragma pack(push,1)
struct R5PassengerPacketPlayerPoolView {
 BYTE prefix[0x1f8a];
 CNetPlayer *players[MAX_PLAYERS];
 R5RemotePassengerUpdateView *GetRemote(PLAYERID id) {
  if(id>MAX_PLAYERS) return NULL;
  CNetPlayer *player=players[id];
  if(!player) return NULL;
  return (R5RemotePassengerUpdateView*)player->m_pRemotePlayer;
 }
};
struct R5PassengerPacketPoolsView {
 void *vehiclePool;
 R5PassengerPacketPlayerPoolView *playerPool;
};
struct R5PassengerPacketNetView {
 BYTE prefix[0x3cd];
 int gameState;
 BYTE unknown3D1[13];
 R5PassengerPacketPoolsView *pools;
 R5PassengerPacketPlayerPoolView *GetPlayerPool() { return pools->playerPool; }
 void PacketPassengerSync(Packet *packet);
};
#pragma pack(pop)
typedef char R5PassengerPacketDataOffset[(offsetof(Packet,data)==16)?1:-1];
typedef char R5PassengerPacketLengthOffset[(offsetof(Packet,length)==8)?1:-1];
typedef char R5PassengerPacketNetPools[(offsetof(R5PassengerPacketNetView,pools)==0x3de)?1:-1];
typedef char R5PassengerPacketSyncSize[(sizeof(R5RemotePassengerSync)==24)?1:-1];
void R5PassengerPacketNetView::PacketPassengerSync(Packet *packet)
{
 R5RemotePassengerUpdateView *player;
 RakNet::BitStream stream((char*)packet->data,packet->length,false);
 BYTE packetId=0;
 PLAYERID playerId;
 R5RemotePassengerSync sync;
 if(gameState!=5) return;
 stream.Read(packetId);
 stream.Read(playerId);
 stream.Read((char*)&sync,sizeof(sync));
 player=GetPlayerPool()->GetRemote(playerId);
 if(player) player->StorePassengerSync(&sync);
}
