// R5 pointer-only selection views, no object allocations or invented providers.
#include <windows.h>
class CNetGame;
extern CNetGame *pNetGame;
class R5TextDrawSelectionView {
    unsigned char unknown0[0x9A7];
    BYTE selectable;
    unsigned char unknown9a8[0x17];
    BYTE drawn;
    BYTE keyCodes;
    RECT bounds;
    BYTE highlighted;
    DWORD highlightColor;
public:
    RECT *GetSelectableRect() { if(selectable && drawn) return &bounds; return NULL; }
    void SetHighlight(bool value,DWORD color) { highlighted=value; highlightColor=color; }
};
class R5TextDrawSelectionPoolView {
    BOOL active[2304];
    R5TextDrawSelectionView *items[2304];
public:
    R5TextDrawSelectionView *GetAt(WORD id) { if(id>=2304) return NULL; if(!active[id]) return NULL; return items[id]; }
};
struct R5TextDrawSelectionPoolsView { unsigned char unknown0[0x1c]; R5TextDrawSelectionPoolView *textdrawPool; };
struct R5TextDrawSelectionNetView { unsigned char unknown0[0x3de]; R5TextDrawSelectionPoolsView *pools; };
class R5TextDrawSelectorView {
    BOOL enabled;
    DWORD highlightColor;
    WORD selectedId;
public:
    void ClearAll();
    void UpdateHover();
    void Enable(DWORD color);
};
//712B0/81
void R5TextDrawSelectorView::ClearAll()
{
    if(pNetGame) {
        R5TextDrawSelectionPoolView *pool=((R5TextDrawSelectionNetView*)pNetGame)->pools->textdrawPool;
        if(pool) {
            int x=0;
            while(x!=2304) {
                R5TextDrawSelectionView *text=pool->GetAt((WORD)x);
                if(text) text->SetHighlight(false,0);
                x++;
            }
        }
    }
}
//71310/251; actual desktop APIs are compiled only, never invoked by evaluation.
void R5TextDrawSelectorView::UpdateHover()
{
    POINT point;
    GetCursorPos(&point);
    ScreenToClient(*(HWND*)0xC97C1C,&point);
    if(pNetGame) {
        R5TextDrawSelectionPoolView *pool=((R5TextDrawSelectionNetView*)pNetGame)->pools->textdrawPool;
        if(pool) {
            int x=0;
            selectedId=0xFFFF;
            while(x!=2304) {
                R5TextDrawSelectionView *text=pool->GetAt((WORD)x);
                if(text) {
                    text->SetHighlight(false,0);
                    RECT *rect=pool->GetAt((WORD)x)->GetSelectableRect();
                    if(rect && PtInRect(rect,point)) {
                        selectedId=(WORD)x;
                        pool->GetAt((WORD)x)->SetHighlight(true,highlightColor);
                    }
                }
                x++;
            }
        }
    }
}
//71440/60, exact observed byte-order exchange expression.
void R5TextDrawSelectorView::Enable(DWORD color)
{
    DWORD upper=(color>>16)|(color&0xFF0000);
    DWORD lower=(color<<16)|(color&0xFF00);
    DWORD converted=(lower<<8)|(upper>>8);
    enabled=TRUE;
    highlightColor=converted;
    selectedId=0xFFFF;
}
