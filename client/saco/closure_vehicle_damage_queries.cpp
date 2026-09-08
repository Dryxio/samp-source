// Source025 complete damage status queries; original R5 only queries car subtype.
#include "main.h"
#include "game/util.h"
struct R5VehicleDamageQueriesView {
 BYTE unknown0[0x4c]; VEHICLE_TYPE *vehicle;
 DWORD GetPanelDamageStatus();
 DWORD GetDoorDamageStatus();
 BYTE GetLightDamageStatus();
};
DWORD R5VehicleDamageQueriesView::GetPanelDamageStatus()
{
	if(vehicle && GetVehicleSubtypeFromVehiclePtr(vehicle)==1) {
		return vehicle->dwPanelStatus;
	}
	return 0;
}

//-----------------------------------------------------------

DWORD R5VehicleDamageQueriesView::GetDoorDamageStatus()
{
	if(vehicle && GetVehicleSubtypeFromVehiclePtr(vehicle)==1) {
		return vehicle->dwDoorStatus1;
	}
	return 0;
}

//-----------------------------------------------------------

BYTE R5VehicleDamageQueriesView::GetLightDamageStatus()
{
	if(vehicle && GetVehicleSubtypeFromVehiclePtr(vehicle)==1) {
		return (BYTE)vehicle->dwLightStatus;
	}
	return 0;
}

