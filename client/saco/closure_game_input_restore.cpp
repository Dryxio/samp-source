// Complete functions from dashr game.cpp. Only compile statically in matching workflow.
#include "main.h"
#include "game/util.h"
extern int iGtaVersion;
extern IDirect3DDevice9 *pD3DDevice;
extern BYTE byteGetKeyStateFunc[5];
extern BYTE byteGetMouseStateCallEU10[5];
extern BYTE byteGetMouseStateCallUSA10[5];
void RestoreMousePositionUpdate();
static inline void KeyInputDIResetMouse()
{
	*(DWORD*)0xB73424 = 0;
	*(DWORD*)0xB73428 = 0;

	_asm mov edx, 0x541BD0
	_asm call edx
}
static inline void KeyInputUpdatePads()
{
	_asm mov edx, 0x541DD0
	_asm call edx
}

void CGame::ProcessInputDisabling()
{
	if(field_61 != 0) return;

	if(!field_65) {
		UnFuck(0x541DF5,5);
		memcpy((PVOID)0x541DF5,byteGetKeyStateFunc,5);

		if(iGtaVersion == GTASA_VERSION_USA10) {
			UnFuck(0x53F417,5);
			memcpy((PVOID)0x53F417,byteGetMouseStateCallUSA10,5);
		} else {
			UnFuck(0x53F417,5);
			memcpy((PVOID)0x53F417,byteGetMouseStateCallEU10,5);
		}

		RestoreMousePositionUpdate();

		UnFuck(0x53F421,4);
		*(BYTE*)0x53F41F = 0x85;
		*(BYTE*)0x53F420 = 0xC0;
		*(BYTE*)0x53F421 = 0x0F;
		*(BYTE*)0x53F422 = 0x8C;

		KeyInputDIResetMouse();
		KeyInputUpdatePads();
		KeyInputDIResetMouse();

		UnFuck(0x6194A0,1);
		*(BYTE*)0x6194A0 = 0xE9;

		pD3DDevice->ShowCursor(FALSE);
		field_65--;
	} else {
		if(field_65 <= 0)
			return;
		field_65 -= 1;
	}
}
