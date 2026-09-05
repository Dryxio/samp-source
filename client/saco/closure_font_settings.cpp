// Generated complete definitions from main.cpp; see tools/prepare_actor_closure.py.
#include <time.h>
#include <math.h>
#include "main.h"
#include "game/util.h"
extern CGame *pGame;
extern CConfig *pConfig;


int GetFontSize()
{
	int size;

	if (pGame->GetScreenWidth() < 1024)
		size = 14;
	else if (pGame->GetScreenWidth() < 1400)
		size = 16;
	else if (pGame->GetScreenWidth() < 1600)
		size = 18;
	else
		size = 20;

	return size + 2 * pConfig->GetIntVariable("fontsize");
}

int GetFontWeight()
{
	int fontweight = pConfig->GetIntVariable("fontweight");
	if(fontweight == 0) return FW_BOLD;

	return fontweight != 1 ? FW_BOLD : FW_NORMAL;
}

char *GetFontFace()
{
	if(pConfig && pConfig->GetStringVariable("fontface"))
	{
		return pConfig->GetStringVariable("fontface");
	}
	return "Arial";
}

int GetDeathWindowFontSize()
{
	int size = 14;
	if (pGame->GetScreenWidth() < 1024)
		size = 12;

	int fontsize = size + 2 * pConfig->GetIntVariable("fontsize");
	if(fontsize < size)
		fontsize = size;
	return fontsize;
}

int GetUIFontSize()
{
	return 2 * pConfig->GetIntVariable("fontsize") + 20;
}
