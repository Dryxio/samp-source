#include "main.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
extern GAME_SETTINGS tSettings;
extern CGame *pGame;
extern CChatWindow *pChatWindow;
class R5GameVehicleCreationView {
 BYTE prefix[0x6e];
 BYTE keepLoadedVehicles[212];
public:
 CVehicle *NewVehicle(int model,float x,float y,float z,float rotation,int creationFlag);
};
void cmdCreateVehicle(PCHAR szCmd)
{
	if(!tSettings.bDebug) return;

	if(!strlen(szCmd)){
		pChatWindow->AddDebugMessage("Usage: /v (vehicle id).");
		return;
	}	
	int iVehicleType = atoi(szCmd);

	if(pGame->IsGameLoaded())
	{
		pGame->RequestModel(iVehicleType);
		pGame->LoadRequestedModels();

		// place this actor near the player.
		CPlayerPed *pPlayer = pGame->FindPlayerPed();

		if(pPlayer) 
		{
			MATRIX4X4 matPlayer;
			pPlayer->GetMatrix(&matPlayer);
			CHAR blank[9] = "";
			sprintf(blank, "TYPE_%d", iVehicleType);
			CVehicle *pTestVehicle = ((R5GameVehicleCreationView*)pGame)->NewVehicle(iVehicleType,
				(matPlayer.pos.X - 5.0f), (matPlayer.pos.Y - 5.0f),
				matPlayer.pos.Z+1.0f, 0.0f, 0);

			pTestVehicle->Add();
			
			/*DWORD dwRet;
			ScriptCommand(&get_active_interior,&dwRet);
			if ((int)dwRet != 0) pTestVehicle->LinkToInterior((int)dwRet);*/

			return;
		}
		else {
			pChatWindow->AddDebugMessage("I couldn't find the player actor.");
			return;
		}
	}
	else {
		pChatWindow->AddDebugMessage("game not loaded.");
	}
}
