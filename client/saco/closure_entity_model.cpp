// Generated complete definitions from game/entity.cpp; see tools/prepare_actor_closure.py.
#include <time.h>
#include <math.h>
#include "main.h"
#include "game/util.h"
extern CGame *pGame;
extern CChatWindow *pChatWindow;
extern int __stdcall ModelInfoLoaded(UINT model);


BOOL CEntity::SetModelIndex(UINT uiModel)
{
	if(!m_pEntity) return FALSE;
	BOOL loadedHere = FALSE;
	int attempts = 0;

	if(!pGame->IsModelLoaded(uiModel) && !ModelInfoLoaded(uiModel)) {
		pGame->RequestModel(uiModel);
		pGame->LoadRequestedModels();
		while(!pGame->IsModelLoaded(uiModel)) {
			Sleep(1);
			if(++attempts > 200) {
				if(pChatWindow) pChatWindow->AddDebugMessage("Warning: Model %u wouldn't load in time!",uiModel);
				return FALSE;
			}
		}
		loadedHere = TRUE;
	}

	DWORD dwThisEntity = (DWORD)m_pEntity;

	_asm {
		mov		esi, dwThisEntity
		mov		edi, uiModel
		mov     edx, [esi]
		mov     ecx, esi
		call    dword ptr [edx+32] ; destroy RW
		mov     eax, [esi]
		mov		edx, edi
		push    edi
		mov     ecx, esi
		mov     word ptr [esi+34], dx
		call    dword ptr [eax+20] ; SetModelIndex
	}

	return loadedHere;
}
