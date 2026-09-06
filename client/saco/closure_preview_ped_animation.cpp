#include "main.h"
#include "game/rwstuff.h"
// Native signature bool onScreen; wrapper preserves full original input word.
void UpdatePreviewClumpAnimations(RpClump *clump,float step,int onScreen)
{
    _asm push onScreen
    _asm push step
    _asm push clump
    _asm mov edx, 0x4D34F0
    _asm call edx
    _asm pop edx
    _asm pop edx
    _asm pop edx
}
class R5PreviewPedAnimationView {
    BYTE unknown0[0x2A4];
    PED_TYPE *ped;
public:
    void UpdateAnimations(float step,int onScreen);
};
void R5PreviewPedAnimationView::UpdateAnimations(float step,int onScreen)
{
    if(ped) {
        RpClump *clump=(RpClump*)((CEntity*)this)->GetRwObject();
        if(clump) UpdatePreviewClumpAnimations(clump,step,onScreen);
    }
}
