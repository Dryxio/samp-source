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
struct R5RemoteVehicleEntryRequestView {
 BYTE unknown0[0x1dd]; CPlayerPed *ped;
 void EnterVehicle(VEHICLEID id,BOOL passenger);
};
#pragma pack(pop)
typedef char RemoteEntryPedOffset[(offsetof(R5RemoteVehicleEntryRequestView,ped)==0x1dd)?1:-1];
void R5RemoteVehicleEntryRequestView::EnterVehicle(VEHICLEID id,BOOL passenger)
{
 R5VehicleGtaIdPoolView *pool=((R5VehicleGtaIdNetView*)pNetGame)->GetVehiclePool();
 CVehicle *vehicle=((RPCVehicleMotionPoolView*)pool)->GetAt(id);
 if(ped && vehicle && !ped->IsInVehicle()) {
  int gtaId=pool->FindGtaIDFromID(id);
  if(gtaId && gtaId!=0xffff) {
   ped->SetKeys(0,0,0);
   ped->EnterVehicle(gtaId,passenger);
  }
 }
}
