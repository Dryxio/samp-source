#include "main.h"
#include <stdlib.h>
#include <string.h>
extern GAME_SETTINGS tSettings;
extern CGame *pGame;
extern CChatWindow *pChatWindow;
class R5PreviewPedModelView {
    BYTE unknown0[0x44];
    DWORD gtaId;
    BYTE unknown48[0x25C];
    PED_TYPE *ped;
public:
    void SetModelIndex(UINT model);
};
void cmdPlayerSkin(PCHAR szCmd)
{
#ifndef _DEBUG
	if(!tSettings.bDebug) return;
#endif

	if(!strlen(szCmd)){	
		pChatWindow->AddDebugMessage("Usage: player_skin (skin number).");
		return;
	}
	int iPlayerSkin = atoi(szCmd);

	if(pGame->IsGameLoaded())
	{
		CPlayerPed *pPlayer = pGame->FindPlayerPed();

		if(pPlayer)
		{
			// Ok, now we can set the model.
			((R5PreviewPedModelView*)pPlayer)->SetModelIndex(iPlayerSkin);
		}
		else
		{
			return;
		}		
	}
	else //! game loaded
	{
		return;
	}
}
