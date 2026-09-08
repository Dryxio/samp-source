// R5 non-owning vehicle view. Timestamp update follows source025;
// local-vehicle predicate and tire mask follow complete original R5 bodies.
#include "main.h"
#include "game/util.h"
struct R5VehicleDriveQueriesView {
 BYTE unknown0[0x4c]; VEHICLE_TYPE *vehicle;
 BYTE unknown50[0x19]; DWORD lastDriven; BOOL hasBeenDriven;
 BOOL IsLocalPlayerVehicle();
 BOOL UpdateLastDrivenTime();
 BYTE GetTireStatus();
};
BOOL R5VehicleDriveQueriesView::IsLocalPlayerVehicle() {
 if(vehicle) {
  PED_TYPE *ped=GamePool_FindPlayerPed();
  if(ped && IN_VEHICLE(ped) && vehicle==(VEHICLE_TYPE*)ped->pVehicle) return TRUE;
 }
 return FALSE;
}
BOOL R5VehicleDriveQueriesView::UpdateLastDrivenTime() {
 if(vehicle && vehicle->pDriver) {
  hasBeenDriven=TRUE;
  lastDriven=GetTickCount();
  return TRUE;
 }
 return FALSE;
}
BYTE R5VehicleDriveQueriesView::GetTireStatus() {
 BYTE status=0;
 if(!vehicle)return 0;
 if(GetVehicleSubtypeFromVehiclePtr(vehicle)==1) {
  if(vehicle->bCarWheelPopped[0])status=1;
  status<<=1;
  if(vehicle->bCarWheelPopped[1])status|=1;
  status<<=1;
  if(vehicle->bCarWheelPopped[2])status|=1;
  status<<=1;
  if(vehicle->bCarWheelPopped[3])status|=1;
 } else if(vehicle && GetVehicleSubtypeFromVehiclePtr(vehicle)==2) {
  if(vehicle->bBikeWheelPopped[0])status=1;
  status<<=1;
  if(vehicle->bBikeWheelPopped[1])status|=1;
 }
 return status;
}
