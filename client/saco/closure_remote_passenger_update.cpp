#include "main.h"
#include <stddef.h>
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
struct R5VehicleGtaIdPoolView { BYTE prefix[0x4fb4]; VEHICLE_TYPE *nativeVehicles[2000]; int FindGtaIDFromID(int id); };
struct R5VehicleGtaIdPoolsView { R5VehicleGtaIdPoolView *vehicles; };
struct R5VehicleGtaIdNetView { BYTE prefix[0x3de]; R5VehicleGtaIdPoolsView *pools; R5VehicleGtaIdPoolView *GetVehiclePool() { return pools->vehicles; } };

// Source025 GetAt uses one combined rejection followed by the success return.
// No new storage or virtual dispatch; this observation view is never allocated.
struct R5RemoteSyncVehiclePoolView : RPCVehicleMotionPoolView {
 CVehicle *GetActiveVehicle(WORD id) {
  if(id>=2000 || !active[id]) return NULL;
  return vehicles[id];
 }
};
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
typedef char R5PassengerSyncSize[(sizeof(R5RemotePassengerSync)==24)?1:-1];
typedef char R5PassengerSyncPosition[(offsetof(R5RemotePassengerSync,position)==12)?1:-1];
typedef char R5PassengerStoredOffset[(offsetof(R5RemotePassengerUpdateView,passengerSync)==0xad)?1:-1];
typedef char R5PassengerStateOffset[(offsetof(R5RemotePassengerUpdateView,state)==0x10a)?1:-1];
typedef char R5PassengerHealthOffset[(offsetof(R5RemotePassengerUpdateView,reportedHealth)==0x1b0)?1:-1];
typedef char R5PassengerLastTickOffset[(offsetof(R5RemotePassengerUpdateView,lastReceivedTick)==0x1b9)?1:-1];
typedef char R5PassengerPedOffset[(offsetof(R5RemotePassengerUpdateView,ped)==0x1dd)?1:-1];
typedef char R5PassengerVehicleOffset[(offsetof(R5RemotePassengerUpdateView,vehicleId)==0x1e7)?1:-1];
void R5RemotePassengerUpdateView::StorePassengerSync(R5RemotePassengerSync *sync)
{
 memcpy(&passengerSync,sync,sizeof(R5RemotePassengerSync));
 vehicleId=sync->vehicleId;
 seat=sync->seatAndFlags & 0x3f;
 R5VehicleGtaIdPoolView *pool=((R5VehicleGtaIdNetView*)pNetGame)->GetVehiclePool();
 vehicle=((R5RemoteSyncVehiclePoolView*)pool)->GetActiveVehicle(vehicleId);
 reportedHealth=(float)sync->health;
 reportedArmour=(float)sync->armour;
 updateType=18;
 lastReceivedTick=GetTickCount();
 afk=0;
 if(ped && !ped->IsInVehicle())
  ((R5RemoteVehicleEntryView*)this)->EnsureVehicleEntry();
 if(state!=18) state=18;
}
