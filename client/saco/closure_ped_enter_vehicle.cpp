#include "main.h"
#include "game/util.h"
extern BOOL r5IgnoreNextVehicleEntry;
extern const SCRIPT_COMMAND r5SelectionPutActorInCarPassenger;
extern const SCRIPT_COMMAND r5SendActorToCarPassenger;
extern const SCRIPT_COMMAND r5SendActorToCarDriver;
void CPlayerPed::EnterVehicle(int iVehicleID, BOOL bPassenger)
{
	if(!m_pPed) return;
	VEHICLE_TYPE *ThisVehicleType;
	if((ThisVehicleType = GamePool_Vehicle_GetAt(iVehicleID)) == NULL) return;
	if(!GamePool_Ped_GetAt(m_dwGTAId)) return;

	r5IgnoreNextVehicleEntry = TRUE;

	if(GetCurrentWeapon() == WEAPON_PARACHUTE) {
		SetArmedWeapon(0,false);
	}

	if(bPassenger) {
		if(ThisVehicleType->entity.nModelIndex == TRAIN_PASSENGER && (m_pPed == GamePool_FindPlayerPed())) {
			ScriptCommand(&r5SelectionPutActorInCarPassenger,m_dwGTAId,iVehicleID,-1);
		} else {
			ScriptCommand(&r5SendActorToCarPassenger,m_dwGTAId,iVehicleID,3000,-1);
		}
	} else {
		ScriptCommand(&r5SendActorToCarDriver,m_dwGTAId,iVehicleID,3000);
	}
}

