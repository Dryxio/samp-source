// Generated complete definitions from game/keystuff.cpp; see tools/prepare_actor_closure.py.
#include <time.h>
#include <math.h>
#include "main.h"
#include "game/util.h"
#include "game/keystuff.h"
extern GTA_CONTROLSET *pGcsInternalKeys;
extern GTA_CONTROLSET GcsRemotePlayerKeys[PLAYER_PED_SLOTS];


GTA_CONTROLSET * GameGetInternalKeys()
{
	return pGcsInternalKeys;
}

GTA_CONTROLSET * GameGetPlayerKeys(int iPlayer)
{
	return &GcsRemotePlayerKeys[iPlayer];
}
