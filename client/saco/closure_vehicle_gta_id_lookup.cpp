#include "main.h"
#include "game/util.h"
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
int R5VehicleGtaIdPoolView::FindGtaIDFromID(int id)
{
 if(nativeVehicles[id]) return GamePool_Vehicle_GetIndex(nativeVehicles[id]);
 return 0xffff;
}
