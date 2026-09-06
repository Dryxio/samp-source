#include "main.h"
#include <stddef.h>
extern CNetGame *pNetGame;
extern CGame *pGame;
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
struct R5VehicleGtaIdPoolView { BYTE prefix[0x4fb4]; VEHICLE_TYPE *nativeVehicles[2000]; int FindGtaIDFromID(int id); };
struct R5VehicleGtaIdPoolsView { R5VehicleGtaIdPoolView *vehicles; };
struct R5VehicleGtaIdNetView { BYTE prefix[0x3de]; R5VehicleGtaIdPoolsView *pools; R5VehicleGtaIdPoolView *GetVehiclePool() { return pools->vehicles; } };

#pragma pack(push,1)
struct R5RemoteVehicleEntryView {
 BYTE unknown0[0x4f];
 BYTE driverWeaponAndFlags;
 BYTE unknown50[0x60];
 BYTE passengerWeaponAndFlags;
 BYTE unknownB1[0x5a];
 BYTE seatId;
 BYTE unknown10C[0xd1];
 CPlayerPed *ped;
 BYTE unknown1E1[6];
 VEHICLEID vehicleId;
 void EnsureVehicleEntry();
};
#pragma pack(pop)

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
typedef char R5DriverSyncSize[(sizeof(R5RemoteDriverSync)==63)?1:-1];
typedef char R5DriverSyncHealth[(offsetof(R5RemoteDriverSync,health)==0x34)?1:-1];
typedef char R5DriverSyncWeapon[(offsetof(R5RemoteDriverSync,weaponAndFlags)==0x36)?1:-1];
typedef char R5DriverStoredOffset[(offsetof(R5RemoteDriverUpdateView,driverSync)==0x19)?1:-1];
typedef char R5DriverStateOffset[(offsetof(R5RemoteDriverUpdateView,state)==0x10a)?1:-1];
typedef char R5DriverHealthOffset[(offsetof(R5RemoteDriverUpdateView,reportedHealth)==0x1b0)?1:-1];
typedef char R5DriverTimestampOffset[(offsetof(R5RemoteDriverUpdateView,lastDriverTimestamp)==0x1bd)?1:-1];
typedef char R5DriverPedOffset[(offsetof(R5RemoteDriverUpdateView,ped)==0x1dd)?1:-1];
typedef char R5DriverVehicleOffset[(offsetof(R5RemoteDriverUpdateView,vehicleId)==0x1e7)?1:-1];
void R5RemoteDriverUpdateView::StateChange(BYTE newState,BYTE oldState)
{
 if(newState==19 && oldState==17) {
  CPlayerPed *localPed=pGame->FindPlayerPed();
  MATRIX4X4 matrix;
  if(localPed && localPed->IsInVehicle() && !localPed->IsAPassenger()) {
   R5VehicleGtaIdPoolView *pool=((R5VehicleGtaIdNetView*)pNetGame)->GetVehiclePool();
   VEHICLEID localVehicle=((RPCVehicleMotionPoolView*)pool)->FindIDFromGtaPtr(localPed->GetGtaVehicle());
   if(localVehicle==vehicleId) {
    localPed->GetMatrix(&matrix);
    localPed->RemoveFromVehicleAndPutAt(matrix.pos.X,matrix.pos.Y,matrix.pos.Z+1.0f);
    pGame->DisplayGameText("~r~Car Jacked~w~!",1000,5);
   }
  }
 }
}
void R5RemoteDriverUpdateView::StoreDriverSync(R5RemoteDriverSync *sync,DWORD timestamp)
{
 if(timestamp && (int)(timestamp-lastDriverTimestamp)<0) return;
 lastDriverTimestamp=timestamp;
 memcpy(&driverSync,sync,sizeof(R5RemoteDriverSync));
 vehicleId=sync->vehicleId;
 R5VehicleGtaIdPoolView *pool=((R5VehicleGtaIdNetView*)pNetGame)->GetVehiclePool();
 vehicle=((RPCVehicleMotionPoolView*)pool)->GetAt(vehicleId);
 seat=0;
 reportedHealth=(float)sync->health;
 reportedArmour=(float)sync->armour;
 updateType=17;
 lastReceivedTick=GetTickCount();
 afk=0;
 if(ped && !ped->IsInVehicle())
  ((R5RemoteVehicleEntryView*)this)->EnsureVehicleEntry();
 if(state!=19) {
  StateChange(19,state);
  state=19;
 }
}
