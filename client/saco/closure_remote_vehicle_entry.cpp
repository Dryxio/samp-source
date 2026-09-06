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

// Pointer-only observation of the existing remote-player allocation.
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
typedef char R5RemoteEntryWeaponOffset[(offsetof(R5RemoteVehicleEntryView,driverWeaponAndFlags)==0x4f)?1:-1];
typedef char R5RemoteEntryPassengerWeaponOffset[(offsetof(R5RemoteVehicleEntryView,passengerWeaponAndFlags)==0xb0)?1:-1];
typedef char R5RemoteEntrySeatOffset[(offsetof(R5RemoteVehicleEntryView,seatId)==0x10b)?1:-1];
typedef char R5RemoteEntryPedOffset[(offsetof(R5RemoteVehicleEntryView,ped)==0x1dd)?1:-1];
typedef char R5RemoteEntryVehicleOffset[(offsetof(R5RemoteVehicleEntryView,vehicleId)==0x1e7)?1:-1];
void R5RemoteVehicleEntryView::EnsureVehicleEntry()
{
 R5VehicleGtaIdPoolView *pool = ((R5VehicleGtaIdNetView*)pNetGame)->GetVehiclePool();
 if(!ped) return;
 if(ped->IsInVehicle()) return;
 CVehicle *vehicle = ((RPCVehicleMotionPoolView*)pool)->GetAt(vehicleId);
 if(vehicle) {
  BYTE weapon = seatId ? passengerWeaponAndFlags : driverWeaponAndFlags;
  weapon &= 0x3f;
  if(ped->GetCurrentWeapon() != weapon) {
   ped->SetArmedWeapon(weapon,false);
   if(ped->GetCurrentWeapon() != weapon) {
    ped->GiveWeapon(weapon,9999);
    ped->SetArmedWeapon(weapon,false);
   }
  }
  int nativeId = pool->FindGtaIDFromID(vehicleId);
  ped->PutDirectlyInVehicle(nativeId,seatId);
 }
}
