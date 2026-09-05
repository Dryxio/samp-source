// Pointer-only selector view, next independent whole function71410/46.
#include "main.h"
extern CGame *pGame;
class R5TextDrawSelectorUpdateView {
    BOOL enabled;
    DWORD highlightColor;
    WORD selectedId;
public:
    void Update();
};
// Token-identical to accepted owner; no new allocation or claimed full layout.
class R5TextDrawSelectorView {
    BOOL enabled;
    DWORD highlightColor;
    WORD selectedId;
public:
    void ClearAll();
    void UpdateHover();
    void Enable(DWORD color);
};
void R5TextDrawSelectorUpdateView::Update()
{
    if(enabled) {
        pGame->ToggleKeyInputsDisabled(2,FALSE);
        pGame->DisplayHud(FALSE);
        ((R5TextDrawSelectorView*)this)->UpdateHover();
    }
}
