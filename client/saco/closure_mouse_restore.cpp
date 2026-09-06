#include "main.h"
#include "game/util.h"
void RestoreMousePositionUpdate()
{
	UnFuck(0x53F47A,4);
	UnFuck(0x53F49A,4);
	UnFuck(0x53F4B3,4);
	*(DWORD*)0x53F47A = 0xB73410;
	*(DWORD*)0x53F49A = 0xB73414;
	*(DWORD*)0x53F4B3 = 0xB7340C;
}
