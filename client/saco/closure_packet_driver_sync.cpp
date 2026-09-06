#include "main.h"
#include <stddef.h>
#pragma pack(push,1)
// Complete observed R5 driver-sync record. Bytes not interpreted by these
// functions remain explicit protocol bytes, not invented layout padding.
struct R5RemoteDriverSync {
 VEHICLEID vehicleId;
 BYTE otherControlsAndMovement[50];
 BYTE health;
 BYTE armour;
 BYTE weaponAndFlags;
 BYTE vehicleExtras[8];
};
struct R5RemoteDriverUpdateView {
 BYTE unknown0[0xc];
 BYTE afk;
 BYTE unknownD[0xc];
 R5RemoteDriverSync driverSync;
 BYTE unknown58[0xb2];
 BYTE state;
 BYTE seat;
 BYTE unknown10C[0xa0];
 float reportedArmour;
 float reportedHealth;
 BYTE unknown1B4[4];
 BYTE updateType;
 DWORD lastReceivedTick;
 DWORD lastDriverTimestamp;
 BYTE unknown1C1[0x1c];
 CPlayerPed *ped;
 CVehicle *vehicle;
 BYTE unknown1E5[2];
 VEHICLEID vehicleId;
 void StateChange(BYTE newState,BYTE oldState);
 void StoreDriverSync(R5RemoteDriverSync *sync,DWORD timestamp);
};
#pragma pack(pop)


#pragma pack(push,1)
struct R5DriverPacketSync {
 VEHICLEID vehicleId;
 WORD leftRightAnalog, upDownAnalog, keys;
 float quaternion[4];
 VECTOR position;
 VECTOR moveSpeed;
 float vehicleHealth;
 BYTE health, armour;
 BYTE weapon : 6;
 BYTE additionalKey : 2;
 BYTE siren, landingGear;
 VEHICLEID trailerId;
 union { DWORD hydraThrust; float trainSpeed; } extra;
};
struct R5DriverPacketPlayerPoolView {
 BYTE prefix[0x1f8a];
 CNetPlayer *players[MAX_PLAYERS];
 R5RemoteDriverUpdateView *GetRemote(PLAYERID id) {
  if(id>MAX_PLAYERS)return NULL;
  CNetPlayer *player=players[id];
  if(!player)return NULL;
  return (R5RemoteDriverUpdateView*)player->m_pRemotePlayer;
 }
};
struct R5DriverPacketPoolsView { void *vehiclePool; R5DriverPacketPlayerPoolView *playerPool; };
struct R5DriverPacketNetView {
 BYTE prefix[0x3cd];
 int gameState;
 BYTE unknown3D1[13];
 R5DriverPacketPoolsView *pools;
 R5DriverPacketPlayerPoolView *GetPlayerPool() { return pools->playerPool; }
 void PacketDriverSync(Packet *packet);
};
#pragma pack(pop)
typedef char DriverPacketSize[(sizeof(R5DriverPacketSync)==63)?1:-1];
typedef char DriverPacketQuat[(offsetof(R5DriverPacketSync,quaternion)==8)?1:-1];
typedef char DriverPacketHealth[(offsetof(R5DriverPacketSync,health)==52)?1:-1];
typedef char DriverPacketExtra[(offsetof(R5DriverPacketSync,extra)==59)?1:-1];
void R5DriverPacketNetView::PacketDriverSync(Packet *packet)
{
 R5RemoteDriverUpdateView *player;
 RakNet::BitStream stream((char*)packet->data,packet->length,false);
 BYTE packetId=0;
 PLAYERID playerId;
 R5DriverPacketSync sync;
 bool siren,landingGear,train,trailer;
 if(gameState!=5)return;
 memset(&sync,0,sizeof(sync));
 DWORD timestamp=0;
 if(packet->data[0]==40) {
  stream.Read(packetId);
  stream.Read(timestamp);
 }
 stream.Read(packetId);
 stream.Read(playerId);
 stream.Read(sync.vehicleId);
 stream.Read(sync.leftRightAnalog);
 stream.Read(sync.upDownAnalog);
 stream.Read(sync.keys);
 stream.ReadNormQuat(sync.quaternion[0],sync.quaternion[1],sync.quaternion[2],sync.quaternion[3]);
 stream.Read((char*)&sync.position,sizeof(VECTOR));
 stream.ReadVector(sync.moveSpeed.X,sync.moveSpeed.Y,sync.moveSpeed.Z);
 WORD vehicleHealth;
 stream.Read(vehicleHealth);
 sync.vehicleHealth=(float)vehicleHealth;
 BYTE healthArmour;
 BYTE armour=0,health=0;
 stream.Read(healthArmour);
 armour=(healthArmour&0xf);
 health=(healthArmour>>4);
 if(armour==15)sync.armour=100;
 else if(armour==0)sync.armour=0;
 else sync.armour=armour*7;
 if(health==15)sync.health=100;
 else if(health==0)sync.health=0;
 else sync.health=health*7;
 BYTE weapon=0;
 stream.Read(weapon);
 sync.weapon=weapon;
 stream.Read(siren);
 if(siren)sync.siren=1;
 stream.Read(landingGear);
 if(landingGear)sync.landingGear=1;
 stream.Read(train);
 if(train)stream.Read(sync.extra.trainSpeed);
 stream.Read(trailer);
 if(trailer)stream.Read(sync.trailerId);
 player=GetPlayerPool()->GetRemote(playerId);
 if(player)player->StoreDriverSync((R5RemoteDriverSync*)&sync,timestamp);
}
