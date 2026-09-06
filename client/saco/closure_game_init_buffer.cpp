// Full InitGame72, not an invented buffer-only replacement.
#include "main.h"
#include "game/keystuff.h"
#include "game/aimstuff.h"
#include "game/hudcolor.h"
extern unsigned char *szGameTextMessage;
extern BOOL ApplyPreGamePatches();
void CGame::InitGame()
{
	// Create a buffer for game text.
	szGameTextMessage = (unsigned char*)calloc(1,513);

	// Init the keystate stuff.
	GameKeyStatesInit();

	// Init the aim stuff.
	GameAimSyncInit();

	// Init radar colors
	GameResetRadarColors();

	if(!ApplyPreGamePatches()) {
		MessageBox(0,
			"I can't determine your GTA version.\r\nSA-MP only supports GTA:SA v1.0 USA/EU",
			"Version Error",MB_OK | MB_ICONEXCLAMATION);
		ExitProcess(1);
	}
}

