// Generated complete definitions from game/game.cpp; see tools/prepare_actor_closure.py.
#include <time.h>
#include "main.h"
#include "game/util.h"


void CGame::RequestModel(int iModelID, int iLoadingStream)
{
	ScriptCommand(&request_model,iModelID);
}

void CGame::LoadRequestedModels()
{
	ScriptCommand(&load_requested_models);
}

BOOL CGame::IsModelLoaded(int iModelID)
{
	if(iModelID > 20000 || iModelID < 0) return TRUE;

	return ScriptCommand(&is_model_available,iModelID);
}

void CGame::SetWorldTime(int iHour, int iMinute)
{
	*(PBYTE)0xB70152 = (BYTE)iMinute;
	*(PBYTE)0xB70153 = (BYTE)iHour;

	ScriptCommand(&set_current_time, iHour, iMinute);
}

void CGame::RequestAnimation(char *szAnimFile)
{
	ScriptCommand(&request_animation, szAnimFile);
}

int CGame::IsAnimationLoaded(char *szAnimFile)
{
	return ScriptCommand(&is_animation_loaded,szAnimFile);
}
