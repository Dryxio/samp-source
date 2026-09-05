// Generated complete definitions from exceptions.cpp; see tools/prepare_actor_closure.py.
#include <time.h>
#include "main.h"
#include "game/util.h"
extern CChatWindow *pChatWindow;
extern DWORD dwScmOpcodeDebug;
extern WORD wVehicleComponentDebug;
int dword_10125A58=0;


int WINAPI exc_filter(unsigned int code, struct _EXCEPTION_POINTERS *ep, char *what)
{
	if(pChatWindow)
	{
		if(!strcmp(what, "opcode"))
		{
			if(dwScmOpcodeDebug == 1767)
			{
				pChatWindow->AddDebugMessage("Warning(add_car_component %u): Exception 0x%X at 0x%X",
					wVehicleComponentDebug, code, ep->ContextRecord->Eip);
				return 1;
			} else {
				pChatWindow->AddDebugMessage("Warning(opcode 0x%X): Exception 0x%X at 0x%X",
					dwScmOpcodeDebug, code, ep->ContextRecord->Eip);
			}
		} else {
			pChatWindow->AddDebugMessage("Warning(%s): Exception 0x%X at 0x%X",
				what, code, ep->ContextRecord->Eip);
		}
	}

	if(dword_10125A58 < 10) {
		dword_10125A58++;
		return 1;
	}
	return 0;
}
