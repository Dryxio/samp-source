// Partial R5 vehicle view, no allocation or invented storage.
#include "main.h"
#include "game/util.h"
#include "game/scripting.h"
__declspec(selectany) extern const SCRIPT_COMMAND r5_car_immunities={0x02ac,"iiiiii"};
__declspec(selectany) extern const SCRIPT_COMMAND r5_tires_vulnerable={0x053f,"ii"};
__declspec(selectany) extern const SCRIPT_COMMAND r5_car_window={0x06ed,"ii"};
struct R5VehicleControlQueriesView {
 BYTE unknown0[0x44]; int gtaId; BYTE unknown48[4]; VEHICLE_TYPE *vehicle;
 BYTE unknown50[8]; BOOL invulnerable;
 void SetInvulnerable(BOOL state);
 void SetHornState(BYTE state);
 void SetWindow(BYTE window);
 BOOL IsValid();
 float GetTrainSpeed();
 void SetTrainSpeed(float speed);
};
// Adapted 0.2.5: R5 excludes trains and unconditionally restores tire vulnerability.
void R5VehicleControlQueriesView::SetInvulnerable(BOOL state) {
 if(!vehicle)return;
 if(!GamePool_Vehicle_GetAt(gtaId))return;
 if(*(DWORD*)vehicle==0x863c40)return;
 if(state) {
  ScriptCommand(&r5_car_immunities,gtaId,1,1,1,1,1);
  ScriptCommand(&r5_tires_vulnerable,gtaId,0);invulnerable=TRUE;
 } else {
  ScriptCommand(&r5_car_immunities,gtaId,0,0,0,0,0);
  ScriptCommand(&r5_tires_vulnerable,gtaId,1);invulnerable=FALSE;
 }
}
void R5VehicleControlQueriesView::SetHornState(BYTE state) {
 if(!vehicle)return;
 if(!GamePool_Vehicle_GetAt(gtaId))return;
 if((!vehicle||GetVehicleSubtypeFromVehiclePtr(vehicle)!=4)&&
    (!vehicle||GetVehicleSubtypeFromVehiclePtr(vehicle)!=5)&&
    (!vehicle||GetVehicleSubtypeFromVehiclePtr(vehicle)!=3)) {
  vehicle->byteHorn=state;vehicle->byteHorn2=state;
 }
}
void R5VehicleControlQueriesView::SetWindow(BYTE window) {
 if(!vehicle)return;
 if(!GamePool_Vehicle_GetAt(gtaId))return;
 if(vehicle&&GetVehicleSubtypeFromVehiclePtr(vehicle)==1 && window<=3)
  ScriptCommand(&r5_car_window,gtaId,(int)window);
}
BOOL R5VehicleControlQueriesView::IsValid(){return GamePool_Vehicle_GetAt(gtaId)!=NULL;}
float R5VehicleControlQueriesView::GetTrainSpeed() {
 if(!vehicle)return 0.0f;
 float speed;
 DWORD value=*(DWORD*)((BYTE*)vehicle+0x5a4);
 memcpy(&speed,&value,4);return speed;
}
void R5VehicleControlQueriesView::SetTrainSpeed(float speed) {
 if(speed>100.0f||speed<-100.0f)return;
 if(vehicle)*(float*)((BYTE*)vehicle+0x5a4)=speed;
}
