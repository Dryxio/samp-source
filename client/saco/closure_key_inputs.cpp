// Generated complete definitions from game/game.cpp; see tools/prepare_actor_closure.py.
#include <time.h>
#include <math.h>
#include "main.h"
#include "game/util.h"
extern IDirect3DDevice9 *pD3DDevice;
void DisableMouseProcess();
void DisableMousePositionUpdate();
void DIResetMouse();
void UpdatePads();


void CGame::ToggleKeyInputsDisabled(int a2, BOOL a3)
{
	if(a2 == 2)
	{
		UnFuck(0x541DF5,5);
		memset((PVOID)0x541DF5,0x90,5);	// disable call

		DisableMouseProcess();
		DIResetMouse();
		UpdatePads();

		UnFuck(0x6194A0u,1);
		*(BYTE*)0x6194A0 = 0xC3;

		pD3DDevice->ShowCursor(TRUE);
		field_61 = 2;
	}
	else if(a2 == 1)
	{
		if(field_61 != 1)
		{
			UnFuck(0x541DF5,5);
			memset((PVOID)0x541DF5,0x90,5);	// disable call

			field_61 = 1;
		}
	}
	else if(a2 == 3)
	{
		if(field_61 != 3)
		{
			DisableMouseProcess();
			DIResetMouse();
			UpdatePads();

			UnFuck(0x6194A0,1);
			*(BYTE*)0x6194A0 = 0xC3;

			pD3DDevice->ShowCursor(TRUE);
			field_61 = 3;
		}
	}
	else if(a2 == 4)
	{
		if(field_61 != 4)
		{
			DisableMousePositionUpdate();
			DIResetMouse();
			UpdatePads();

			UnFuck(0x6194A0,1);
			*(BYTE*)0x6194A0 = 0xC3;

			field_61 = 4;
		}
	}
	else
	{
		if(!a2 && field_61)
		{
			field_65 = a3 != 0 ? 0 : 10;
			pD3DDevice->ShowCursor(FALSE);
			field_61 = 0;
		}
	}
}
