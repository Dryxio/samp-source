#include "main.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
extern CGame *pGame;
extern CChatWindow *pChatWindow;

void cmdShowInterior(PCHAR szCmd)
{
	DWORD dwRet;
	ScriptCommand(&get_active_interior,&dwRet);
	pChatWindow->AddDebugMessage("Current Interior: %u",dwRet);
}
