// 0.2.5 VehicleVelocity/FindIDFromGtaPtr, adapted to verified R5 protocol/layout.
#include "main.h"
extern CGame *pGame;
extern CNetGame *pNetGame;
struct RPCVehicleMotionPoolView {
 int lastVehicleId;
 BYTE unknown4[0x1130];
 CVehicle *vehicles[2000];
 BOOL active[2000];
 VEHICLE_TYPE *nativeVehicles[2000];
 WORD FindIDFromGtaPtr(VEHICLE_TYPE *native);
 CVehicle *GetAt(WORD id) {
  if(id>=2000)return NULL;
  if(active[id])return vehicles[id];
  return NULL;
 }
};
WORD RPCVehicleMotionPoolView::FindIDFromGtaPtr(VEHICLE_TYPE *native) {
 for(int i=1;i<=lastVehicleId;i++)if(native==nativeVehicles[i])return i;
 return 0xffff;
}
struct RPCVehicleMotionPoolsView { RPCVehicleMotionPoolView *vehicles; };
struct RPCVehicleMotionNetView { BYTE unknown[0x3de]; RPCVehicleMotionPoolsView *pools; };
void ScrSetVehicleVelocityTransfer(RPCParameters *rpcParams) {
 PCHAR Data=reinterpret_cast<PCHAR>(rpcParams->input);
 int bits=rpcParams->numberOfBitsOfData;
 RakNet::BitStream stream(Data,(bits/8)+1,false);
 BYTE mode=0;
 VECTOR velocity;
 stream.Read(mode);
 stream.Read(velocity.X);stream.Read(velocity.Y);stream.Read(velocity.Z);
 RPCVehicleMotionPoolView *pool=((RPCVehicleMotionNetView*)pNetGame)->pools->vehicles;
 CPlayerPed *ped=pGame->FindPlayerPed();
 if(ped && ped->IsInVehicle()) {
  WORD id=pool->FindIDFromGtaPtr(ped->GetGtaVehicle());
  if(id!=0xffff) {
   CVehicle *vehicle=pool->GetAt(id);
   if(vehicle) {
    if(mode==0)vehicle->SetMoveSpeedVector(velocity);
    else if(mode==1)vehicle->SetTurnSpeedVector(velocity);
   }
  }
 }
}
