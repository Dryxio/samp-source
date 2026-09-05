// Isolated 0.2.5 player candidates, dc9eb80d80e26cf91e2b92697b8918208f052542.
#include "main.h"
#include "game/util.h"
extern CGame *pGame;
const SCRIPT_COMMAND select_interior					= { 0x04BB, "i" };		// INTERIOR_*
const SCRIPT_COMMAND restart_if_wasted_at				= { 0x016C, "ffffi" };
const SCRIPT_COMMAND link_actor_to_interior				= { 0x0860, "ii" };
class EvaluationPlayerPed : public CPlayerPed { public:
void SetAmmo(BYTE byteWeapon, WORD wordAmmo);
BOOL IsOnGround();
void SetWeaponModelIndex(int iWeapon);
void SetInterior(BYTE byteID);
void RestartIfWastedAt(VECTOR *vecRestart, float fRotation);
};
void EvaluationPlayerPed::SetAmmo(BYTE byteWeapon, WORD wordAmmo)
{
	if(m_pPed)
	{
		//WEAPON_SLOT_TYPE * WeaponSlot = GetCurrentWeaponSlot();
		WEAPON_SLOT_TYPE * WeaponSlot = FindWeaponSlot((DWORD)byteWeapon);
		if(!WeaponSlot) return;
		WeaponSlot->dwAmmo = (DWORD)wordAmmo;
		//WeaponSlot->dwAmmoInClip = 0;
	}
}

BOOL EvaluationPlayerPed::IsOnGround()
{
	if(m_pPed) {
		if(m_pPed->dwStateFlags & 3) {
			return TRUE;
		}
	}
	return FALSE;
}

void EvaluationPlayerPed::SetWeaponModelIndex(int iWeapon)
{
	DWORD dwWeaponInfo;

	if(m_pPed) {
		DWORD dwPed = (DWORD)m_pPed;
		dwWeaponInfo = pGame->GetWeaponInfo(iWeapon,1);
		
		_asm mov ebx, dwWeaponInfo
		_asm mov eax, [ebx+12]

		_asm push eax
		_asm mov ecx, dwPed
		_asm mov edx, 0x5E3990
		_asm call edx
	}
}

void EvaluationPlayerPed::SetInterior(BYTE byteID)
{
	if(!m_pPed) return;

	ScriptCommand(&select_interior,byteID);
	ScriptCommand(&link_actor_to_interior,m_dwGTAId,byteID);

	MATRIX4X4 mat;
	GetMatrix(&mat);
	ScriptCommand(&refresh_streaming_at,mat.pos.X,mat.pos.Y);
}

void EvaluationPlayerPed::RestartIfWastedAt(VECTOR *vecRestart, float fRotation)
{	
	ScriptCommand(&restart_if_wasted_at, vecRestart->X,vecRestart->Y,vecRestart->Z,fRotation,0);
}
