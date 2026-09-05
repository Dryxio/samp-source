// Generated complete definitions from game/game.cpp; see tools/prepare_actor_closure.py.
#include <time.h>
#include <math.h>
#include "main.h"
#include "game/util.h"


void DIResetMouse()
{
	*(DWORD*)0xB73424 = 0;
	*(DWORD*)0xB73428 = 0;

	_asm mov edx, 0x541BD0
	_asm call edx
}

void UpdatePads()
{
	_asm mov edx, 0x541DD0
	_asm call edx
}
