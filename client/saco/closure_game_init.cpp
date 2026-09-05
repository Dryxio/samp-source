// Generated complete definitions from game/game.cpp; see tools/prepare_actor_closure.py.
#include <time.h>
#include <math.h>
#include "main.h"
#include "game/util.h"
extern BOOL bUsedPlayerSlots[PLAYER_PED_SLOTS];


CGame::CGame()
{
	m_pGameAudio = new CAudio();
	m_pGameCamera = new CCamera();
	m_pGamePlayer = NULL;
	m_bCheckpointsEnabled = FALSE;
	m_bRaceCheckpointsEnabled = FALSE;
	m_dwRaceCheckpointHandle = NULL;
	field_61 = 0;
	field_65 = 0;
	field_69 = FALSE;
	field_6D = 0;
	memset(bUsedPlayerSlots, 0, sizeof(bUsedPlayerSlots));
	memset(field_6E, 0, sizeof(field_6E));
	field_55 = 0;
	field_59 = 1;
	field_5D = 90;
}

BYTE CGame::FindFirstFreePlayerPedSlot()
{
	BYTE x=2;
	while(x!=PLAYER_PED_SLOTS) {
		if(bUsedPlayerSlots[x] != TRUE) return x;
		x++;
	}
	return 0;
}

BYTE CGame::FUNC_100A00F0()
{
	BYTE count = 0;
	BYTE x = 2;
	while(x!=PLAYER_PED_SLOTS) {
		if(bUsedPlayerSlots[x] == TRUE) {
			count++;
		}
		x++;
	}
	return count;
}
