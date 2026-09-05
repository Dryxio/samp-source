// Generated complete definitions from game/util.cpp; see tools/prepare_actor_closure.py.
#include <time.h>
#include <math.h>
#include "main.h"
#include "game/util.h"
#undef PI
#define PI 3.14159265f


void __stdcall WorldAddEntity(DWORD *dwEnt)
{
	_asm push dwEnt
	_asm mov ebx, 0x563220
	_asm call ebx
	_asm pop ebx
}

void __stdcall WorldRemoveEntity(DWORD *dwEnt)
{
	_asm push dwEnt
	_asm mov ebx, 0x563280
	_asm call ebx
	_asm pop ebx
}

PED_TYPE * __stdcall GamePool_Ped_GetAt(int iID)
{
	PED_TYPE *pActorRet;

	_asm mov ebx, ADDR_PED_TABLE
	_asm mov ecx, [ebx]
	_asm push iID
	_asm mov ebx, ADDR_ACTOR_FROM_ID
	_asm call ebx
	_asm mov pActorRet, eax

	return pActorRet;	
}

float DegToRad(float fDegrees)
{
	if (fDegrees > 360.0f || fDegrees < 0.0f) return 0.0f;
	if (fDegrees > 180.0f) {
		return (float)(-(PI - (((fDegrees - 180.0f) * PI) / 180.0f)));
	} else {
		return (float)((fDegrees * PI) / 180.0f);
	}
}
