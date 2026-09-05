// Generated complete definitions from game/aimstuff.cpp; see tools/prepare_actor_closure.py.
#include <time.h>
#include <math.h>
#include "main.h"
#include "game/util.h"
extern CAMERA_AIM caRemotePlayerAim[PLAYER_PED_SLOTS];
extern float *pfStats;
extern float fLocalWeaponSkill[11];
extern float fWeaponSkill[PLAYER_PED_SLOTS][11];


void __stdcall GameStoreRemotePlayerAim(int iPlayer, CAMERA_AIM * caAim)
{
	memcpy(&caRemotePlayerAim[iPlayer],caAim,sizeof(CAMERA_AIM));
}

void __stdcall GameSetPlayerWeaponSkillLevel(int iPlayer, int iSkill, WORD wLevel)
{
	if(iSkill < 11 && iPlayer < PLAYER_PED_SLOTS) {
		fWeaponSkill[iPlayer][iSkill] = (float)wLevel;
	}
}

void __stdcall GameSetLocalPlayerWeaponSkillLevel(int iSkill, WORD wLevel)
{
	if(iSkill < 11) {
		fLocalWeaponSkill[iSkill] = (float)wLevel;
		pfStats[iSkill + 69] = (float)wLevel;
	}
}

void __stdcall GameStoreLocalPlayerWeaponSkills()
{
	fLocalWeaponSkill[0] = pfStats[69]; // pistol
	fLocalWeaponSkill[1] = pfStats[70]; // silenced pistol
	fLocalWeaponSkill[2] = pfStats[71]; // desert eagle
	fLocalWeaponSkill[3] = pfStats[72]; // shotgun
	fLocalWeaponSkill[4] = pfStats[73]; // sawnoff
	fLocalWeaponSkill[5] = pfStats[74]; // spas12
	fLocalWeaponSkill[6] = pfStats[75]; // micro uzi
	fLocalWeaponSkill[7] = pfStats[76]; // mp5
	fLocalWeaponSkill[8] = pfStats[77]; // ak47
	fLocalWeaponSkill[9] = pfStats[78]; // m4
	fLocalWeaponSkill[10] = pfStats[79]; // sniper
}

void __stdcall GameSetLocalPlayerWeaponSkills()
{
	pfStats[69] = fLocalWeaponSkill[0]; // pistol
	pfStats[70] = fLocalWeaponSkill[1]; // silenced pistol
	pfStats[71] = fLocalWeaponSkill[2]; // desert eagle
	pfStats[72] = fLocalWeaponSkill[3]; // shotgun
	pfStats[73] = fLocalWeaponSkill[4]; // sawnoff
	pfStats[74] = fLocalWeaponSkill[5]; // spas12
	pfStats[75] = fLocalWeaponSkill[6]; // micro uzi
	pfStats[76] = fLocalWeaponSkill[7]; // mp5
	pfStats[77] = fLocalWeaponSkill[8]; // ak47
	pfStats[78] = fLocalWeaponSkill[9]; // m4
	pfStats[79] = fLocalWeaponSkill[10]; // sniper
}

void __stdcall GameSetRemotePlayerWeaponSkills(int iPlayer)
{
	pfStats[69] = fWeaponSkill[iPlayer][0]; // pistol
	pfStats[70] = fWeaponSkill[iPlayer][1]; // silenced pistol
	pfStats[71] = fWeaponSkill[iPlayer][2]; // desert eagle
	pfStats[72] = fWeaponSkill[iPlayer][3]; // shotgun
	pfStats[73] = fWeaponSkill[iPlayer][4]; // sawnoff
	pfStats[74] = fWeaponSkill[iPlayer][5]; // spas12
	pfStats[75] = fWeaponSkill[iPlayer][6]; // micro uzi
	pfStats[76] = fWeaponSkill[iPlayer][7]; // mp5
	pfStats[77] = fWeaponSkill[iPlayer][8]; // ak47
	pfStats[78] = fWeaponSkill[iPlayer][9]; // m4
	pfStats[79] = fWeaponSkill[iPlayer][10]; // sniper
}
