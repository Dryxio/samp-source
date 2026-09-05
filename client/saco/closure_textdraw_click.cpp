// Whole selection click/send/disable candidates, no execution or live network transmission.
#include "main.h"
extern CNetGame *pNetGame;
extern CGame *pGame;
extern CCmdWindow *pCmdWindow;
extern const int r5RpcClickTextdraw;
// Token-identical declaration to the existing selector view; definitions remain in its owner TU.
class R5TextDrawSelectorView {
    BOOL enabled;
    DWORD highlightColor;
    WORD selectedId;
public:
    void ClearAll();
    void UpdateHover();
    void Enable(DWORD color);
};
class R5TextDrawClickView {
    BOOL enabled;
    DWORD highlightColor;
    WORD selectedId;
public:
    void SendClick();
    void Disable();
    BOOL MsgProc(UINT message,WPARAM wParam,LPARAM lParam);
};
//71480/151 plus full exception data, which must be retained/reviewed.
void R5TextDrawClickView::SendClick()
{
    RakNet::BitStream stream;
    // Use the actual WORD-by-value RakNet API; its inline parameter gets
    // the compiler ABI temporary rather than an aliased integer local.
    stream.Write<WORD>(selectedId);
    pNetGame->GetRakClient()->RPC((char*)&r5RpcClickTextdraw,&stream,HIGH_PRIORITY,RELIABLE_ORDERED,0,FALSE);
}
//71520/77. CmdWindow active flag14E0 is actual verified CCmdWindow layout.
void R5TextDrawClickView::Disable()
{
    enabled=FALSE;
    pGame->ToggleKeyInputsDisabled(0,FALSE);
    if(pCmdWindow->m_bEnabled) {
        pCmdWindow->Disable();
        pCmdWindow->Enable();
    }
    selectedId=0xFFFF;
    SendClick();
    ((R5TextDrawSelectorView*)this)->ClearAll();
}
//71570/41. WM_LBUTTONUP and invalid-id behavior exactly as R5.
BOOL R5TextDrawClickView::MsgProc(UINT message,WPARAM wParam,LPARAM lParam)
{
    if(enabled && message==WM_LBUTTONUP) {
        if(selectedId!=0xFFFF) SendClick();
        return TRUE;
    }
    return FALSE;
}
