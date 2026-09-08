// R5 model predicates and light state; source0.2.5 supplies RC/light methods.
#include "main.h"
#include "game/util.h"
struct R5VehicleTypeLightsView {
 BYTE unknown0[0x44]; int gtaId; BYTE unknown48[4]; VEHICLE_TYPE *vehicle;
 BOOL IsTrailerModel(); BOOL IsTowTruck(); BOOL IsRCVehicle();
 void ToggleLights(BYTE enabled); bool AreLightsEnabled();
};
BOOL R5VehicleTypeLightsView::IsTrailerModel() {
 if(!vehicle)return FALSE;
 if(!GamePool_Vehicle_GetAt(gtaId))return FALSE;
 if(vehicle->entity.nModelIndex==435||vehicle->entity.nModelIndex==450||vehicle->entity.nModelIndex==584||vehicle->entity.nModelIndex==591||vehicle->entity.nModelIndex==606||vehicle->entity.nModelIndex==607||vehicle->entity.nModelIndex==608||vehicle->entity.nModelIndex==610||vehicle->entity.nModelIndex==611)return TRUE;
 return FALSE;
}
BOOL R5VehicleTypeLightsView::IsTowTruck() {
 if(!vehicle)return FALSE;
 if(!GamePool_Vehicle_GetAt(gtaId))return FALSE;
 return vehicle->entity.nModelIndex==525;
}
BOOL R5VehicleTypeLightsView::IsRCVehicle() {
 if(!vehicle)return FALSE;
 if(!GamePool_Vehicle_GetAt(gtaId))return FALSE;
 if(vehicle->entity.nModelIndex==441||vehicle->entity.nModelIndex==464||vehicle->entity.nModelIndex==465||vehicle->entity.nModelIndex==594||vehicle->entity.nModelIndex==501||vehicle->entity.nModelIndex==564)return TRUE;
 return FALSE;
}
void R5VehicleTypeLightsView::ToggleLights(BYTE enabled) {
 if(!vehicle)return;
 if(!GamePool_Vehicle_GetAt(gtaId))return;
 if(!enabled) {
  *((BYTE*)vehicle+0x4a8)|=8;
  *((BYTE*)vehicle+0x428)&=~64;
 } else {
  *((BYTE*)vehicle+0x4a8)&=~8;
  *((BYTE*)vehicle+0x428)|=64;
 }
}
bool R5VehicleTypeLightsView::AreLightsEnabled() {
 if(!vehicle)return true;
 if(!GamePool_Vehicle_GetAt(gtaId))return true;
 struct LightBits { unsigned char low:3; unsigned char enabled:1; unsigned char high:4; };
 return ((LightBits*)((BYTE*)vehicle+0x4a8))->enabled;
}
