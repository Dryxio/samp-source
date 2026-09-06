#include "main.h"
typedef char R5LocalPlayerAllocationSize[(sizeof(CLocalPlayer)==0x324)?1:-1];
typedef char R5PlayerPoolSizeVerified[(sizeof(CPlayerPool)==0x2F3E)?1:-1];
// Normal C++ bounds qualification reconstructed from original setter-shaped loop.
CPlayerPool::CPlayerPool()
{

	
	// loop through and initialize all net players to null and slot states to false
	for(PLAYERID playerId = 0; playerId < MAX_PLAYERS; playerId++) {
		if (playerId <= MAX_PLAYERS) m_pPlayers[playerId] = NULL;
		field_2A[playerId] = 0;
	}

	m_pLocalPlayer = new CLocalPlayer();

	field_0 = 0;
	field_22 = 0;
	m_LocalPlayerID = 0;
	field_2F3A = 0;
}

//----------------------------------------------------
