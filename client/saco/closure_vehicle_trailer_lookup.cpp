// 0.2.5 trailer lookup adapted to R5 subtype exclusions and invalid-id sentinel.
#include "main.h"
#include "game/util.h"
extern CNetGame *pNetGame;
struct R5VehicleTrailerPoolView { CVehicle *GetAt(WORD id); BOOL GetSlotState(WORD id); };
struct RPCVehicleMotionPoolView { WORD FindIDFromGtaPtr(VEHICLE_TYPE *vehicle); };
struct R5VehicleTrailerPoolsView { R5VehicleTrailerPoolView *vehicles; };
struct R5VehicleTrailerNetView { BYTE unknown0[0x3de]; R5VehicleTrailerPoolsView *pools; };
struct R5VehicleTrailerLookupView {
 BYTE unknown0[0x4c]; VEHICLE_TYPE *vehicle;
 CVehicle *GetTrailer();
};
CVehicle *R5VehicleTrailerLookupView::GetTrailer() {
 if(!vehicle)return NULL;
 UINT subtype=GetVehicleSubtypeFromVehiclePtr(vehicle);
 if(subtype==2||subtype==4||subtype==7||subtype==6)return NULL;
 VEHICLE_TYPE *trailer=*(VEHICLE_TYPE**)((BYTE*)vehicle+0x4c4);
 if(trailer&&pNetGame) {
  R5VehicleTrailerPoolView *pool=((R5VehicleTrailerNetView*)pNetGame)->pools->vehicles;
  WORD id=((RPCVehicleMotionPoolView*)pool)->FindIDFromGtaPtr(trailer);
  if(id!=0xffff && pool->GetSlotState(id))return pool->GetAt(id);
 }
 return NULL;
}
