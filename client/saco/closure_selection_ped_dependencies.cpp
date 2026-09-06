#include "main.h"
#include "game/util.h"
extern CGame *pGame;
extern CNetGame *pNetGame;
extern const SCRIPT_COMMAND r5SelectionTogglePlayerControllable;
extern const SCRIPT_COMMAND r5SelectionLockActor;
extern const SCRIPT_COMMAND r5SelectionPutActorInCarDriver;
extern const SCRIPT_COMMAND r5SelectionPutActorInCarPassenger;
extern const SCRIPT_COMMAND r5SelectionCameraOnVehicle;
struct RPCVehicleMotionPoolView {
 int lastVehicleId; BYTE unknown4[0x1130]; CVehicle *vehicles[2000];
 BOOL active[2000]; VEHICLE_TYPE *nativeVehicles[2000];
 WORD FindIDFromGtaPtr(VEHICLE_TYPE *native);
 CVehicle *GetAt(WORD id) { if(id>=2000)return NULL; if(active[id])return vehicles[id]; return NULL; }
};
// Read-only partial view; never allocated or used as a complete pool object.
struct R5VehicleTrailerPoolView {
 BYTE unknown0[0x1134]; CVehicle *vehicles[2000]; BOOL active[2000];
 __declspec(dllexport) CVehicle *GetAt(WORD id);
 __declspec(dllexport) BOOL GetSlotState(WORD id);
};

struct R5VehicleTrailerPoolsView { R5VehicleTrailerPoolView *vehicles; };
struct R5VehicleTrailerNetView { BYTE unknown0[0x3de]; R5VehicleTrailerPoolsView *pools; };
enum { R5InvalidVehicleId=0xffff };
void CPlayerPed::TogglePlayerControllable(int iControllable)
{
	MATRIX4X4 mat;

	if(!GamePool_Ped_GetAt(m_dwGTAId)) return;

	if(!iControllable) {
		ScriptCommand(&r5SelectionTogglePlayerControllable,m_bytePlayerNumber,0);
		ScriptCommand(&r5SelectionLockActor,m_dwGTAId,1);
	} else {
		ScriptCommand(&r5SelectionTogglePlayerControllable,m_bytePlayerNumber,1);
		ScriptCommand(&r5SelectionLockActor,m_dwGTAId,0);
		if(!m_pPed || !IN_VEHICLE(m_pPed)) {
			GetMatrix(&mat);
			TeleportTo(mat.pos.X,mat.pos.Y,mat.pos.Z);
		}
	}
}
void CPlayerPed::PutDirectlyInVehicle(int iVehicleID, int iSeat)
{
	if(!m_pPed) return;
	if(!GamePool_Vehicle_GetAt(iVehicleID)) return;
	if(!GamePool_Ped_GetAt(m_dwGTAId)) return;

	if(GetCurrentWeapon() == WEAPON_PARACHUTE) {
		SetArmedWeapon(0,false);
	}

	VEHICLE_TYPE *pVehicle = GamePool_Vehicle_GetAt(iVehicleID);

	if(pVehicle->fHealth == 0.0f) return;

	// Check to make sure internal data structure of the vehicle hasn't been deleted
	// by checking if the vtbl points to CPlaceable_vtbl
	if (pVehicle->entity.vtable == 0x863C40) return;

	if ((GetVehicleSubtypeFromVehiclePtr(pVehicle) == VEHICLE_SUBTYPE_CAR ||
		GetVehicleSubtypeFromVehiclePtr(pVehicle) == VEHICLE_SUBTYPE_BIKE) &&
		iSeat > pVehicle->byteMaxPassengers)
	{
		return;
	}

	if(iSeat==0) {
		if(pVehicle->pDriver && IN_VEHICLE(pVehicle->pDriver)) return;
		ScriptCommand(&r5SelectionPutActorInCarDriver,m_dwGTAId,iVehicleID);
	} else {
		iSeat--;
		ScriptCommand(&r5SelectionPutActorInCarPassenger,m_dwGTAId,iVehicleID,iSeat);
	}
	if(m_pPed == GamePool_FindPlayerPed() && IN_VEHICLE(m_pPed)) {
		pGame->GetCamera()->SetBehindPlayer();
	}

	if(pNetGame) {
		R5VehicleTrailerPoolView* pVehiclePool = ((R5VehicleTrailerNetView*)pNetGame)->pools->vehicles;
		VEHICLEID TrainVehicleId = ((RPCVehicleMotionPoolView*)pVehiclePool)->FindIDFromGtaPtr(pVehicle);
		if(TrainVehicleId == R5InvalidVehicleId || TrainVehicleId > MAX_VEHICLES) return;

		CVehicle* pTrain = pVehiclePool->GetAt(TrainVehicleId);
		if ( pTrain && pTrain->IsATrainPart() && m_pPed == GamePool_FindPlayerPed() ) {
			ScriptCommand(&r5SelectionCameraOnVehicle, pTrain->m_dwGTAId, 3, 2);
		}
	}
}
