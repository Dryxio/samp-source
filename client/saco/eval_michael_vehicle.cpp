// Isolated 0.2.5 vehicle candidates, dc9eb80d80e26cf91e2b92697b8918208f052542.
#include "main.h"
#include "game/util.h"
class EvaluationVehicle : public CVehicle { public:
void SetTankRot(float X, float Y);
float GetTankRotX();
float GetTankRotY();
float GetTrainSpeed();
void SetTrainSpeed(float fSpeed);
void SetWheelPopped(DWORD wheelid, DWORD popped);
BYTE GetWheelPopped(DWORD wheelid);
void UpdateDamageStatus(DWORD dwPanelDamage, DWORD dwDoorDamage, BYTE byteLightDamage);
DWORD GetPanelDamageStatus();
DWORD GetDoorDamageStatus();
BYTE GetLightDamageStatus();
};
void EvaluationVehicle::SetTankRot(float X, float Y)
{
	m_pVehicle->fTankRotX = X;
	m_pVehicle->fTankRotY = Y;
}

float EvaluationVehicle::GetTankRotX()
{
	return m_pVehicle->fTankRotX;
}

float EvaluationVehicle::GetTankRotY()
{
	return m_pVehicle->fTankRotY;
}

float EvaluationVehicle::GetTrainSpeed()
{
	return m_pVehicle->fTrainSpeed;
}

void EvaluationVehicle::SetTrainSpeed(float fSpeed)
{
	m_pVehicle->fTrainSpeed = fSpeed;
}

void EvaluationVehicle::SetWheelPopped(DWORD wheelid, DWORD popped)
{
	if (GetVehicleSubtype() == VEHICLE_SUBTYPE_CAR)
		m_pVehicle->bCarWheelPopped[wheelid] = (BYTE)popped;
	else if (GetVehicleSubtype() == VEHICLE_SUBTYPE_BIKE)
		m_pVehicle->bBikeWheelPopped[wheelid] = (BYTE)popped;
}

BYTE EvaluationVehicle::GetWheelPopped(DWORD wheelid)
{
	if (GetVehicleSubtype() == VEHICLE_SUBTYPE_CAR)
		return m_pVehicle->bCarWheelPopped[wheelid];
	else if (GetVehicleSubtype() == VEHICLE_SUBTYPE_BIKE)
		return m_pVehicle->bBikeWheelPopped[wheelid];
	return 0;
}

void EvaluationVehicle::UpdateDamageStatus(DWORD dwPanelDamage, DWORD dwDoorDamage, BYTE byteLightDamage)
{
	if(m_pVehicle && GetVehicleSubtype() == VEHICLE_SUBTYPE_CAR) {

		if(!dwPanelDamage && !dwDoorDamage && !byteLightDamage) {
			if(m_pVehicle->dwPanelStatus || m_pVehicle->dwDoorStatus1 || m_pVehicle->dwLightStatus) {
				// The ingame car is damaged in some way although the update
				// says the car should be repaired. So repair it and exit.
				DWORD dwVehiclePtr = (DWORD)m_pVehicle;
				_asm mov ecx, dwVehiclePtr
				_asm mov edx, 0x6A3440 // CAutomobile::RepairDamageModel
				_asm call edx

				return;
			}
		}

		m_pVehicle->dwPanelStatus = dwPanelDamage;
		m_pVehicle->dwDoorStatus1 = dwDoorDamage;
		m_pVehicle->dwLightStatus = (DWORD)byteLightDamage;

		DWORD dwVehiclePtr = (DWORD)m_pVehicle;
		_asm mov ecx, dwVehiclePtr
		_asm mov edx, 0x6B3E90 // CAutomobile::UpdateDamageModel
		_asm call edx		
	}
}

DWORD EvaluationVehicle::GetPanelDamageStatus()
{
	if(m_pVehicle && GetVehicleSubtype() == VEHICLE_SUBTYPE_CAR) {
		return m_pVehicle->dwPanelStatus;
	}
	return 0;
}

DWORD EvaluationVehicle::GetDoorDamageStatus()
{
	if(m_pVehicle && GetVehicleSubtype() == VEHICLE_SUBTYPE_CAR) {
		return m_pVehicle->dwDoorStatus1;
	}
	return 0;
}

BYTE EvaluationVehicle::GetLightDamageStatus()
{
	if(m_pVehicle && GetVehicleSubtype() == VEHICLE_SUBTYPE_CAR) {
		return (BYTE)m_pVehicle->dwLightStatus;
	}
	return 0;
}
