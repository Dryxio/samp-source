// Five RPCs unlocked by accepted FindPlayerPed/lifetime. No new providers.
#include "main.h"
#include <string.h>
extern CGame *pGame;
// Direct 0.2.5 function transfer, adapted only to real R5 char* BitStream ctor.
void ScrSetPlayerVelocityTransfer(RPCParameters *rpcParams) {
 PCHAR Data=reinterpret_cast<PCHAR>(rpcParams->input);
 int bits=rpcParams->numberOfBitsOfData;
 RakNet::BitStream stream(Data,(bits/8)+1,false);
 VECTOR velocity;
 stream.Read(velocity.X); stream.Read(velocity.Y); stream.Read(velocity.Z);
 CPlayerPed *ped=pGame->FindPlayerPed();
 if(ped) {
  if(ped->IsOnGround()) {
   DWORD flags=ped->GetStateFlags(); flags^=3; ped->SetStateFlags(flags);
  }
  ped->SetMoveSpeedVector(velocity);
 }
}
void ScrSetPlayerDrunkVisualsTransfer(RPCParameters *rpcParams) {
 PCHAR Data=reinterpret_cast<PCHAR>(rpcParams->input);
 int bits=rpcParams->numberOfBitsOfData;
 RakNet::BitStream stream(Data,(bits/8)+1,false);
 int visuals; stream.Read(visuals);
 CPlayerPed *ped=pGame->FindPlayerPed();
 if(ped)ScriptCommand(&set_player_drunk_visuals,0,visuals);
}
void ScrSetPlayerDrunkHandlingTransfer(RPCParameters *rpcParams) {
 PCHAR Data=reinterpret_cast<PCHAR>(rpcParams->input);
 int bits=rpcParams->numberOfBitsOfData;
 RakNet::BitStream stream(Data,(bits/8)+1,false);
 int visuals; stream.Read(visuals);
 CPlayerPed *ped=pGame->FindPlayerPed();
 if(ped)ScriptCommand(&handling_responsiveness,0,visuals);
}
// R5 reconstruction only: these two RPC implementations do not exist in 0.2.5.
void ScrSetPlayerShopNameTransfer(RPCParameters *rpcParams) {
 PCHAR Data=reinterpret_cast<PCHAR>(rpcParams->input);
 int bits=rpcParams->numberOfBitsOfData;
 RakNet::BitStream stream(Data,(bits/8)+1,false);
 char shop[33]; memset(shop,0,sizeof(shop));
 stream.Read(shop,32);
 CPlayerPed *ped=pGame->FindPlayerPed();
 if(ped) {
  int length=strlen(shop);
  if(length==0) {
   ped->SetShopName(NULL);
   ped->LoadShoppingDataSubsection("");
  } else {
   ped->SetShopName(shop);
   ped->LoadShoppingDataSubsection(shop);
  }
 }
}
void ScrSetPlayerArmedWeaponTransfer(RPCParameters *rpcParams) {
 PCHAR Data=reinterpret_cast<PCHAR>(rpcParams->input);
 int bits=rpcParams->numberOfBitsOfData;
 RakNet::BitStream stream(Data,(bits/8)+1,false);
 CPlayerPed *ped=pGame->FindPlayerPed();
 if(!ped)return;
 int weapon; stream.Read(weapon);
 if(weapon>=0 && weapon<=46)ped->SetArmedWeapon(weapon,false);
}
