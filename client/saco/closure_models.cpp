// Generated complete definitions from game/game.cpp; see tools/prepare_actor_closure.py.
#include <time.h>
#include <math.h>
#include "main.h"
#include "game/util.h"
// Original game.cpp draw-zone callback declaration.
typedef void (*DrawZone_t)(float *fPos, DWORD *dwColor, BYTE byteMenu);


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

void CGame::sub_100A0090(int a1, int a2)
{
	if(a1 && a2 && a1 < 1000 / a2)
		Sleep(1000 / a2 - a1 - 1);
}

float CGame::FindGroundZForCoord(float x, float y, float z)
{
	float fGroundZ;
	ScriptCommand(&get_ground_z, x, y, z, &fGroundZ);
	return fGroundZ;
}

BOOL CGame::IsGameLoaded()
{
	if(!(*(PBYTE)ADDR_GAME_STARTED)) return TRUE;
	return FALSE;
}

void CGame::GetWorldTime(BYTE* byteHour, BYTE* byteMinute)
{
	*byteMinute = *(PBYTE)0xB70152;
	*byteHour = *(PBYTE)0xB70153;
}

void CGame::ToggleThePassingOfTime(BYTE byteOnOff)
{
	UnFuck(0x52CF10,1);

	if(byteOnOff) {
		*(PBYTE)0x52CF10 = 0x56; // push esi
		field_69 = TRUE;
	}
	else {
		*(PBYTE)0x52CF10 = 0xC3; // ret
		field_69 = FALSE;
	}
}

void CGame::SetWorldWeather(int iWeatherID)
{
	if(field_69) {
		*(DWORD*)(0xC81318) = iWeatherID;
	} else {
		*(DWORD*)(0xC81318) = iWeatherID;
		*(DWORD*)(0xC8131C) = iWeatherID;
		*(DWORD*)(0xC81320) = iWeatherID;
	}
}

int CGame::GetWorldWeather()
{
	return *(int*)0xC81318;
}

BYTE CGame::IsHudEnabled()
{
	return *(BYTE*)ADDR_ENABLE_HUD;
}

BOOL CGame::IsFrameLimiterEnabled()
{
	if(*(PBYTE)0xBA6794) return TRUE;
	return FALSE;
}

void CGame::EnableFrameLimiter()
{
	*(BYTE*)0xBA6794 = 1;
}

void CGame::SetFrameLimit(DWORD dwLimit)
{
	field_5D = dwLimit;

	UnFuck(0xC1704C,4);
	*(DWORD*)0xC1704C = 200;
}

void CGame::SetMaxStats()
{
	// driving stat
	_asm mov eax, 0x4399D0
	_asm call eax

	// weapon stats
	_asm mov eax, 0x439940
	_asm call eax

	// Disable CStats::SetStatValue
	UnFuck(0x55A070,1);
	*(BYTE*)0x55A070 = 0xC3;
}

void CGame::DisableTrainTraffic()
{
	ScriptCommand(&enable_train_traffic,0);
}

void CGame::RefreshStreamingAt(float x, float y)
{
	ScriptCommand(&refresh_streaming_at,x,y);
}

void CGame::ToggleRadar(int iToggle)
{
	*(PBYTE)0xBAA3FB = (BYTE)!iToggle;
}

void CGame::DisableRaceCheckpoint()
{
	if (m_dwRaceCheckpointHandle)
	{
		ScriptCommand(&destroy_racing_checkpoint, m_dwRaceCheckpointHandle);
		m_dwRaceCheckpointHandle = NULL;
	}
	m_bRaceCheckpointsEnabled = false;
}

DWORD CGame::CreateRadarMarkerIcon(int iMarkerType, float fX, float fY, float fZ, DWORD dwColor, int iStyle)
{
	DWORD dwMarkerID=0;

	if(iStyle == MAPICON_LOCAL) {
		ScriptCommand(&create_radar_marker_without_sphere, fX, fY, fZ, iMarkerType, &dwMarkerID);
	} else if(iStyle == MAPICON_GLOBAL) {
		ScriptCommand(&create_marker_at, fX, fY, fZ, iMarkerType, &dwMarkerID);
	} else if(iStyle == MAPICON_LOCAL_CHECKPOINT) {
		ScriptCommand(&create_radar_marker_icon, fX, fY, fZ, iMarkerType, &dwMarkerID);
	} else if(iStyle == MAPICON_GLOBAL_CHECKPOINT) {
		ScriptCommand(&create_icon_marker_sphere, fX, fY, fZ, iMarkerType, &dwMarkerID);
	} else {
		ScriptCommand(&create_radar_marker_without_sphere, fX, fY, fZ, iMarkerType, &dwMarkerID);
	}

	if(iMarkerType == 0) {
		if(dwColor < 1004) {
			ScriptCommand(&set_marker_color, dwMarkerID, dwColor);
			ScriptCommand(&show_on_radar, dwMarkerID, 2);
		} else {
			ScriptCommand(&set_marker_color, dwMarkerID, dwColor);
			ScriptCommand(&show_on_radar, dwMarkerID, 3);
		}
	}
	return dwMarkerID;
}

BYTE CGame::GetActiveInterior()
{
	DWORD dwRet;
	ScriptCommand(&get_active_interior,&dwRet);
	return (BYTE)dwRet;
}

void CGame::AddToLocalMoney(int iAmount)
{
	ScriptCommand(&add_to_player_money,0,iAmount);
}

int CGame::GetLocalMoney()
{
	return *(int *)0xB7CE50;
}

void CGame::RestartEverything()
{
	//*(PBYTE)ADDR_MENU = 1;
	*(PBYTE)ADDR_MENU2 = 1;
	*(PBYTE)ADDR_MENU3 = 1;

	//(PBYTE)ADDR_GAME_STARTED = 0;
	//*(PBYTE)ADDR_MENU = 1;

	OutputDebugString("ShutDownForRestart");
	_asm mov edx, 0x53C550 ; internal_CGame_ShutDownForRestart
	_asm call edx

	OutputDebugString("Timers stopped");
	_asm mov edx, 0x561AA0 ; internal_CTimer_Stop
	_asm call edx

	OutputDebugString("ReInitialise");
	_asm mov edx, 0x53C680 ; internal_CGame_InitialiseWhenRestarting
	_asm call edx

	*(PBYTE)ADDR_GAME_STARTED = 1;
}

DWORD CGame::GetWeaponInfo(int iWeapon, int iUnk)
{
	DWORD dwRet;

	_asm push iUnk
	_asm push iWeapon
	_asm mov edx, 0x743C60
	_asm call edx
	_asm pop ecx
	_asm pop ecx
	_asm mov dwRet, eax

	return dwRet;
}

void CGame::SetGravity(float fGravity)
{
	UnFuck(0x863984, 4);
	*(float*)0x863984 = fGravity;
}

void CGame::SetWantedLevel(BYTE byteLevel)
{
	*(BYTE*)0x58DB60 = byteLevel;
}

void CGame::SetGameTextCount(WORD wCount)
{
	*(WORD*)0xA44B68 = wCount;
}

void CGame::DrawGangZone(float fPos[], DWORD dwColor)
{
	((DrawZone_t)0x5853D0)(fPos, &dwColor, *(BYTE*)ADDR_MENU);
}

void CGame::EnableZoneNames(BYTE byteEnable)
{
	ScriptCommand(&enable_zone_names, byteEnable);
}

void CGame::EnableStuntBonus(bool bEnable)
{
	UnFuck(0xA4A474,4);
	*(DWORD*)0xA4A474 = (int)bEnable;
}

void CGame::DisableEnterExits()
{
	DWORD pEnExPool = *(DWORD *)0x96A7D8;
	DWORD pEnExEntries = *(DWORD *)pEnExPool;

	int iNumEnEx=0;
	int x=0;

	_asm mov ecx, pEnExPool
	_asm mov eax, [ecx+8]
	_asm mov iNumEnEx, eax

	BYTE *pEnExPoolSlot;
	while(x!=iNumEnEx) {
		pEnExPoolSlot = (((BYTE *)pEnExEntries) + (60*x));
		_asm mov eax, pEnExPoolSlot
		_asm and word ptr [eax+48], 0
		x++;
	}   
}

void CGame::LoadScene(char* szScene)
{
	_asm push szScene
	_asm mov ebx, 0x5B8700
	_asm call ebx
	_asm pop ebx
}

int CGame::GetMemoryUsedForStreaming()
{
	return *(int*)0x8E4CB4;
}

int CGame::GetMemoryAvailableForStreaming()
{
	return *(int*)0x8A5A80;
}

int CGame::GetLoadedVehicleModelCount()
{
	int iCount = 0;

	struct DUMMY {
		PADDING(_pad0,16); // 0-16
		BOOL bLoaded; // 16-20
	};

	DUMMY *pStreamingModelInfo = (DUMMY*)0x8E4CC0;
	int x = 400;
	while(x != 611) {
		if(pStreamingModelInfo[x].bLoaded)
			iCount++;
		x++;
	}
	return iCount;
}

DWORD CGame::GetTimeInMilliseconds()
{
	return *(DWORD*)0xB7CB84;
}

int CGame::GetRwObjectsCount()
{
	return *(int*)0xB71804;
}

void CGame::LoadCollisionFile(char *szFileName)
{
	_asm push 0
	_asm push szFileName
	_asm mov edx, 0x5B4E60
	_asm call edx
	_asm pop edx
	_asm pop edx
}

void CGame::LoadCullZone(char *szLine)
{
	_asm push szLine
	_asm mov edx, 0x5B4B40
	_asm call edx
	_asm pop edx
}

BOOL CGame::IsUsingController()
{
	if(*(BYTE*)0xBA6818) return TRUE;
	return FALSE;
}

void CGame::DisableWeaponLockOnTarget()
{
	// CPlayerPed_FindWeaponLockOnTarget
	UnFuck(0x60DC50,1);
	*(BYTE*)0x60DC50 = 0xC3; // ret

	// CPlayerPed_FindNextWeaponLockOnTarget
	UnFuck(0x60E530,3);
	*(BYTE*)0x60E530 = 0xC2; // retn 8
	*(BYTE*)0x60E531 = 0x08;
	*(BYTE*)0x60E532 = 0x00;
}

int CGame::GetWeaponModelIDFromWeapon(int iWeaponID)
{
	return GameGetWeaponModelIDFromWeaponID(iWeaponID);
}


