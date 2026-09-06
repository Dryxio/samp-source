#include "main.h"
extern CNetGame *pNetGame;
extern BOOL bQuitGame;
extern DWORD dwStartQuitTick;
void QuitGame()
{
	if(pNetGame && pNetGame->GetGameState() == GAMESTATE_CONNECTED) {
		pNetGame->GetRakClient()->Disconnect(500);
	}
	bQuitGame = TRUE;
	dwStartQuitTick = GetTickCount();
}
