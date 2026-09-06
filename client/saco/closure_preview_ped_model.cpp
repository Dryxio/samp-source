// R5 AFF50/113, source0.2.5 CPlayerPed::SetModelIndex adapted to current layout.
#include "main.h"
#include "game/util.h"
BOOL __stdcall IsActorPedModelValid(int);
void UnFuck(DWORD,int);
class R5PreviewPedModelView {
    BYTE unknown0[0x44];
    DWORD gtaId;
    BYTE unknown48[0x25C];
    PED_TYPE *ped;
public:
    void SetModelIndex(UINT model);
};
void R5PreviewPedModelView::SetModelIndex(UINT model)
{
    DWORD nativePed=(DWORD)ped;
    if(!GamePool_Ped_GetAt(gtaId)) return;
    if(!IsActorPedModelValid(model)) model=0;
    if(nativePed) {
        UnFuck(0x5A82C0,1);
        *(BYTE*)0x5A82C0=0xC3;
        ((CPlayerPed*)this)->DestroyFollowPedTask();
        ((CEntity*)this)->SetModelIndex(model);
        // Original native CAEPedAudioEntity initialization, ped audio member294.
        _asm mov eax, nativePed
        _asm lea ecx, [eax+0x294]
        _asm push eax
        _asm mov ebx, 0x4E68D0
        _asm call ebx
    }
}
