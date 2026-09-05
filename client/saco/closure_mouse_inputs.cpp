// Generated complete definitions from game/game.cpp; see tools/prepare_actor_closure.py.
#include <time.h>
#include <math.h>
#include "main.h"
#include "game/util.h"
extern DWORD dwDummyActiveMouseState;


void DisableMouseProcess()
{
	UnFuck(0x53F417,5);
	memset((PVOID)0x53F417,0x90,5);

	UnFuck(0x53F41F,4);
	*(BYTE*)0x53F41F = 0x33;
	*(BYTE*)0x53F420 = 0xC0;
	*(BYTE*)0x53F421 = 0x0F;
	*(BYTE*)0x53F422 = 0x84;
}

void DisableMousePositionUpdate()
{
	memset((PVOID)0xB7340C,0,12);

	UnFuck(0x53F47A,4);
	UnFuck(0x53F49A,4);
	UnFuck(0x53F4B3,4);
	*(DWORD*)0x53F47A = (DWORD)&dwDummyActiveMouseState;
	*(DWORD*)0x53F49A = (DWORD)&dwDummyActiveMouseState;
	*(DWORD*)0x53F4B3 = (DWORD)&dwDummyActiveMouseState;
}
