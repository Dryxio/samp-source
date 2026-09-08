// Source025 damage model with R5 car-or-plane condition. Native GTA calls retained.
#include "main.h"
#include "game/util.h"
struct R5VehicleDamageView {
 BYTE unknown0[0x4c]; VEHICLE_TYPE *vehicle;
 void UpdateDamageStatus(DWORD panels,DWORD doors,BYTE lights);
};
void R5VehicleDamageView::UpdateDamageStatus(DWORD dwPanelDamage, DWORD dwDoorDamage, BYTE byteLightDamage)
{
	if(vehicle && (GetVehicleSubtypeFromVehiclePtr(vehicle)==1 || (vehicle && GetVehicleSubtypeFromVehiclePtr(vehicle)==5))) {

		if(!dwPanelDamage && !dwDoorDamage && !byteLightDamage) {
			if(vehicle->dwPanelStatus || vehicle->dwDoorStatus1 || vehicle->dwLightStatus) {
				// The ingame car is damaged in some way although the update
				// says the car should be repaired. So repair it and exit.
				DWORD dwVehiclePtr = (DWORD)vehicle;
				_asm mov ecx, dwVehiclePtr
				_asm mov edx, 0x6A3440 // CAutomobile::RepairDamageModel
				_asm call edx

				return;
			}
		}

		vehicle->dwPanelStatus = dwPanelDamage;
		vehicle->dwDoorStatus1 = dwDoorDamage;
		vehicle->dwLightStatus = (DWORD)byteLightDamage;

		DWORD dwVehiclePtr = (DWORD)vehicle;
		_asm mov ecx, dwVehiclePtr
		_asm mov edx, 0x6B3E90 // CAutomobile::UpdateDamageModel
		_asm call edx		
	}
}

