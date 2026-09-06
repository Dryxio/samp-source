#include "main.h"
#pragma pack(push,1)
struct R5RemoteVehicleExitRequestView {
 BYTE unknown0[0x1dd]; CPlayerPed *ped;
 void ExitVehicle();
};
#pragma pack(pop)
void R5RemoteVehicleExitRequestView::ExitVehicle()
{
 if(ped && ped->IsInVehicle()) {
  ped->SetKeys(0,0,0);
  ped->ExitCurrentVehicle();
 }
}
