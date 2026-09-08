// Original base source native GTA landing-gear bridge, symbolic inline assembly.
// No copied instruction bytes. Partial view only; actual subtype provider.
#include "main.h"
#include "game/util.h"
struct R5VehicleLandingGearView {
 BYTE unknown0[0x4c]; VEHICLE_TYPE *m_pVehicle;
 UINT GetVehicleSubtype(){return m_pVehicle?GetVehicleSubtypeFromVehiclePtr(m_pVehicle):0;}
 void SetLandingGearState(eLandingGearState state);
 eLandingGearState GetLandingGearState();
};
void R5VehicleLandingGearView::SetLandingGearState(eLandingGearState state)
{
	if(GetVehicleSubtype() != VEHICLE_SUBTYPE_PLANE) return;

	DWORD dwVehiclePtr = (DWORD)m_pVehicle;
	float fPlaneLandingGear = 0.0f;

	_asm mov eax, dwVehiclePtr
	_asm mov edx, [eax+0x9CC]
	_asm mov fPlaneLandingGear, edx

	if (state == LGS_DOWN && fPlaneLandingGear == 0.0f)
	{
		_asm mov ecx, dwVehiclePtr
		_asm mov edx, 0x6CAC20
		_asm call edx
	}
	else if(state == LGS_UP && fPlaneLandingGear == 1.0f)
	{
		_asm mov ecx, dwVehiclePtr
		_asm mov edx, 0x6CAC70
		_asm call edx		
	}
}

//-----------------------------------------------------------

eLandingGearState R5VehicleLandingGearView::GetLandingGearState()
{
	if(GetVehicleSubtype() != VEHICLE_SUBTYPE_PLANE) return LGS_UP;

	DWORD dwVehiclePtr = (DWORD)m_pVehicle;
	float fPlaneLandingGear = 0.0f;

	_asm mov eax, dwVehiclePtr
	_asm mov edx, [eax+0x9CC]
	_asm mov fPlaneLandingGear, edx

	if(fPlaneLandingGear == 0.0f) return LGS_UP;
	return LGS_DOWN;
}

//-----------------------------------------------------------

