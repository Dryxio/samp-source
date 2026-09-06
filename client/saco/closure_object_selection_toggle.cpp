// Verified object-selection controller26EB64: init alloc6 -> ctor6DA00.
// Pointer view only; no allocation or claim that the complete editor is reconstructed.
#include "main.h"
extern CGame *pGame;
extern CCmdWindow *pCmdWindow;
struct R5InputRestoreCountdownView { BYTE unknown[0x65]; int countdown; };
class R5ObjectSelectionView {
    BOOL enabled;
    WORD selectedObjectId;
public:
    R5ObjectSelectionView();
    void Toggle(BOOL enable);
};
R5ObjectSelectionView::R5ObjectSelectionView()
{
    enabled=FALSE;
    selectedObjectId=0xFFFF;
}
//6DB80/117. Preserve second condition re-read after calls.
void R5ObjectSelectionView::Toggle(BOOL enable)
{
    if(enabled && !enable) {
        pGame->ToggleKeyInputsDisabled(enable,TRUE);
        if(pCmdWindow->m_bEnabled) {
            pCmdWindow->Disable();
            pCmdWindow->Enable();
        }
    }
    if(!enabled && enable) {
        pGame->ToggleKeyInputsDisabled(0,TRUE);
        ((R5InputRestoreCountdownView*)pGame)->countdown=0;
        pGame->ProcessInputDisabling();
    }
    enabled=enable;
}
