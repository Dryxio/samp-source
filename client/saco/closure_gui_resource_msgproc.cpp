// Isolated R5 GUI proposal.
// Source: client/saco/eval_gui_full.cpp, lines 307-317.
// Only the non-virtual resource-manager message root is staged here; the
// blocked IME EditBox MsgProc family is deliberately not included.
#include "d3d9/common/dxstdafx.h"

bool CDXUTDialogResourceManager::MsgProc( HWND hWnd, UINT uMsg,
                                          WPARAM wParam, LPARAM lParam )
{
    // Let the CDXUTIMEEditBox's static message proc handle the msg.
    // This is because some IME messages must be handled to ensure
    // proper functionalities and the static msg proc ensures that
    // this happens even if no control has the input focus.
    if( CDXUTIMEEditBox::StaticMsgProc( uMsg, wParam, lParam ) )
        return true;

    return false;
}
