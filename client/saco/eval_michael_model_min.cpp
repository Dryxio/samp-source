#include "main.h"
#include "game/util.h"
#include <time.h>
extern CGame *pGame;
extern CChatWindow *pChatWindow;
class EvaluationEntity : public CEntity { public: void SetModelIndex(UINT uiModel); };
void EvaluationEntity::SetModelIndex(UINT uiModel)
{
	if(!m_pEntity) return;

	if(!pGame->IsModelLoaded(uiModel)) {
		pGame->RequestModel(uiModel);
		pGame->LoadRequestedModels();
		while(!pGame->IsModelLoaded(uiModel)) Sleep(1);
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

	//pGame->RemoveModel(uiModel);
}
