// Generated complete definitions from game/util.cpp; see tools/prepare_actor_closure.py.
#include <time.h>
#include <math.h>
#include "main.h"
#include "game/util.h"
extern DWORD dwPlayerPedPtrs[PLAYER_PED_SLOTS];
extern struc_13 VAR_1026C258[PLAYER_PED_SLOTS];


void __stdcall InitPlayerPedPtrRecords()
{
	memset(&dwPlayerPedPtrs[0],0,sizeof(DWORD) * PLAYER_PED_SLOTS);
	memset(&VAR_1026C258[0],0,sizeof(struc_13) * PLAYER_PED_SLOTS);
}

void __stdcall SetPlayerPedPtrRecord(BYTE bytePlayer, DWORD dwPedPtr)
{
	dwPlayerPedPtrs[bytePlayer] = dwPedPtr;
	memset(&VAR_1026C258[bytePlayer], 0, sizeof(struc_13));
}

DWORD __stdcall GetPlayerPedPtrRecord(BYTE bytePlayer)
{
	return dwPlayerPedPtrs[bytePlayer];
}

BYTE __stdcall FindPlayerNumFromPedPtr(DWORD dwPedPtr)
{
	BYTE x = 0;
	while(x != PLAYER_PED_SLOTS)
	{
		if(dwPlayerPedPtrs[x] == dwPedPtr) return x;
		x++;
	}
	return 0;
}

void __stdcall FUNC_100B4390(BYTE bytePlayer, DWORD a2, DWORD a3, DWORD a4, DWORD a5)
{
	if(bytePlayer < PLAYER_PED_SLOTS)
	{
		VAR_1026C258[bytePlayer].field_0 = a2;
		VAR_1026C258[bytePlayer].field_4 = a3;
		VAR_1026C258[bytePlayer].field_8 = a4;
		VAR_1026C258[bytePlayer].field_C = a5;
	}
}

struc_13 * __stdcall FUNC_100B43D0(BYTE bytePlayer)
{
	if(bytePlayer < PLAYER_PED_SLOTS)
	{		
		return &VAR_1026C258[bytePlayer];
	}
	return NULL;
}

BYTE __stdcall FUNC_100B43F0(DWORD *pdwRenderWare)
{
	BYTE x = 0;
	while(x != PLAYER_PED_SLOTS)
	{
		ENTITY_TYPE *pEntity = (ENTITY_TYPE *)dwPlayerPedPtrs[x];
		if(pEntity)
		{
			if(pEntity->pdwRenderWare)
			{
				if(pEntity->pdwRenderWare == pdwRenderWare)
					return x;
			}
		}
		x++;
	}
	return 0;
}
