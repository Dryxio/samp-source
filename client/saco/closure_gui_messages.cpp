// R5 message routing with complete original focus/hover methods.
#include "d3d9/common/dxstdafx.h"
#include <new>
#ifndef WM_XBUTTONDOWN
#define WM_XBUTTONDOWN 0x020B // (not always defined)
#endif
#ifndef WM_XBUTTONUP
#define WM_XBUTTONUP 0x020C // (not always defined)
#endif
#ifndef WM_MOUSEWHEEL
#define WM_MOUSEWHEEL 0x020A // (not always defined)
#endif
#ifndef WHEEL_DELTA
#define WHEEL_DELTA 120 // (not always defined)
#endif

bool CDXUTDialog::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    bool bHandled = false;

	//OutputDebugString("CDXUTDialog::MsgProc");

    // For invisible dialog, do not handle anything.
    if( !m_bVisible )
        return false;

    // If automation command-line switch is on, enable this dialog's keyboard input
    // upon any key press or mouse click.
    if( DXUTGetAutomation() &&
        ( WM_LBUTTONDOWN == uMsg || WM_LBUTTONDBLCLK == uMsg || WM_KEYDOWN == uMsg ) )
    {
        m_bKeyboardInput = true;
    }

    // If the dialog is minimized, don't send any messages to controls.
    if( m_bMinimized )
        return false;

    // If a control is in focus, it belongs to this dialog, and it's enabled, then give
    // it the first chance at handling the message.
    if( s_pControlFocus && 
        s_pControlFocus->m_pDialog == this && 
        s_pControlFocus->GetEnabled() )
    {
        // If the control MsgProc handles it, then we don't.
        if( s_pControlFocus->MsgProc( uMsg, wParam, lParam ) )
            return true;
    }

    switch( uMsg )
    {
        case WM_SIZE:
        case WM_MOVE:
        {
            // Handle sizing and moving messages so that in case the mouse cursor is moved out
            // of an UI control because of the window adjustment, we can properly
            // unhighlight the highlighted control.
            POINT pt = { -1, -1 };
            OnMouseMove( pt );
            break;
        }

        case WM_ACTIVATEAPP:
            // Call OnFocusIn()/OnFocusOut() of the control that currently has the focus
            // as the application is activated/deactivated.  This matches the Windows
            // behavior.
            if( s_pControlFocus && 
                s_pControlFocus->m_pDialog == this && 
                s_pControlFocus->GetEnabled() )
            {
                if( wParam )
                    s_pControlFocus->OnFocusIn();
                else
                    s_pControlFocus->OnFocusOut();
            }
            break;

        // Keyboard messages
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
        case WM_KEYUP:
        case WM_SYSKEYUP:
        {
            // If a control is in focus, it belongs to this dialog, and it's enabled, then give
            // it the first chance at handling the message.
            if( s_pControlFocus && 
                s_pControlFocus->m_pDialog == this && 
                s_pControlFocus->GetEnabled() )
            {
                if( s_pControlFocus->HandleKeyboard( uMsg, wParam, lParam ) )
                    return true;
            }

            // Not yet handled, see if this matches a control's hotkey
            // Activate the hotkey if the focus doesn't belong to an
            // edit box.
            if( uMsg == WM_KEYDOWN && ( !s_pControlFocus ||
                                      ( s_pControlFocus->GetType() != DXUT_CONTROL_EDITBOX
                                     && s_pControlFocus->GetType() != DXUT_CONTROL_IMEEDITBOX ) ) )
            {
                for( int i=0; i < m_Controls.GetSize(); i++ )
                {
                    CDXUTControl* pControl = m_Controls.GetAt( i );
                    if( pControl->GetHotkey() == wParam )
                    {
                        pControl->OnHotkey();
                        return true;
                    }
                }
            }

            // Not yet handled, check for focus messages
            if( uMsg == WM_KEYDOWN )
            {
                // If keyboard input is not enabled, this message should be ignored
                if( !m_bKeyboardInput )
                    return false;

                switch( wParam )
                {
                    case VK_RIGHT:
                    case VK_DOWN:
                        if( s_pControlFocus != NULL )
                        {
                            return OnCycleFocus( true );
                        }
                        break;

                    case VK_LEFT:
                    case VK_UP:
                        if( s_pControlFocus != NULL )
                        {
                            return OnCycleFocus( false );
                        }
                        break;

                    case VK_TAB: 
                        if( s_pControlFocus == NULL )
                        {
                            FocusDefaultControl();
                        }
                        else
                        {
                            bool bShiftDown = ((GetAsyncKeyState( VK_SHIFT ) & 0x8000) != 0);
                            return OnCycleFocus( !bShiftDown );
                        }
                        return true;
                }
            }

            break;
        }


        // Mouse messages
        case WM_MOUSEMOVE:
        case WM_LBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_MBUTTONDOWN:
        case WM_MBUTTONUP:
        case WM_RBUTTONDOWN:
        case WM_RBUTTONUP:
        case WM_XBUTTONDOWN:
        case WM_XBUTTONUP:
        case WM_LBUTTONDBLCLK:
        case WM_MBUTTONDBLCLK:
        case WM_RBUTTONDBLCLK:
        case WM_MOUSEWHEEL:
        {


            // If not accepting mouse input, return false to indicate the message should still 
            // be handled by the application (usually to move the camera).
            if( !m_bMouseInput )
                return false;

            POINT mousePoint = { short(LOWORD(lParam)), short(HIWORD(lParam)) };
            mousePoint.x -= m_x;
            mousePoint.y -= m_y;

            // If caption is enabled, offset the Y coordinate by the negative of its height.
            if( m_bCaption )
                mousePoint.y -= m_nCaptionHeight;

            // If a control is in focus, it belongs to this dialog, and it's enabled, then give
            // it the first chance at handling the message.
            if( s_pControlFocus && 
                s_pControlFocus->m_pDialog == this && 
                s_pControlFocus->GetEnabled() )
            {
                if( s_pControlFocus->HandleMouse( uMsg, mousePoint, wParam, lParam ) )
                    return true;
            }

            // Not yet handled, see if the mouse is over any controls
            CDXUTControl* pControl = GetControlAtPoint( mousePoint );
            if( pControl != NULL && pControl->GetEnabled() )
            {
                bHandled = pControl->HandleMouse( uMsg, mousePoint, wParam, lParam );
                if( bHandled )
                    return true;
            }
            else
            {
                // Mouse not over any controls in this dialog, if there was a control
                // which had focus it just lost it
                if( uMsg == WM_LBUTTONDOWN && 
                    s_pControlFocus && 
                    s_pControlFocus->m_pDialog == this )
                {
                    s_pControlFocus->OnFocusOut();
                    s_pControlFocus = NULL;
                }
            }

            // Still not handled, hand this off to the dialog. Return false to indicate the
            // message should still be handled by the application (usually to move the camera).
            switch( uMsg )
            {
                case WM_MOUSEMOVE:
                    OnMouseMove( mousePoint );
                    return false;
            }

            break;
        }
    }

    return false;
}
CDXUTControl* CDXUTDialog::GetControlAtPoint( POINT pt )
{
    // Search through all child controls for the first one which
    // contains the mouse point
    for( int i=0; i < m_Controls.GetSize(); i++ )
    {
        CDXUTControl* pControl = m_Controls.GetAt(i);

        if( pControl == NULL )
        {
            continue;
        }

        // We only return the current control if it is visible
        // and enabled.  Because GetControlAtPoint() is used to do mouse
        // hittest, it makes sense to perform this filtering.
        if( pControl->ContainsPoint( pt ) && pControl->GetEnabled() && pControl->GetVisible() )
        {
            return pControl;
        }
    }

    return NULL;
}
void CDXUTDialog::OnMouseMove( POINT pt )
{
    // Figure out which control the mouse is over now
    CDXUTControl* pControl = GetControlAtPoint( pt );

    // If the mouse is still over the same control, nothing needs to be done
    if( pControl == m_pControlMouseOver )
        return;

    // Handle mouse leaving the old control
    if( m_pControlMouseOver )
        m_pControlMouseOver->OnMouseLeave();

    // Handle mouse entering the new control
    m_pControlMouseOver = pControl;
    if( pControl != NULL )
        m_pControlMouseOver->OnMouseEnter();
}
bool CDXUTDialog::OnCycleFocus( bool bForward )
{
    // This should only be handled by the dialog which owns the focused control, and 
    // only if a control currently has focus
    if( s_pControlFocus == NULL || s_pControlFocus->m_pDialog != this )
        return false;

    CDXUTControl* pControl = s_pControlFocus;
    for( int i=0; i < 0xffff; i++ )
    {
        pControl = (bForward) ? GetNextControl( pControl ) : GetPrevControl( pControl );
        
        // If we've gone in a full circle then focus doesn't change
        if( pControl == s_pControlFocus )
            return true;

        // If the dialog accepts keybord input and the control can have focus then
        // move focus
        if( pControl->m_pDialog->m_bKeyboardInput && pControl->CanHaveFocus() )
        {
            s_pControlFocus->OnFocusOut();
            s_pControlFocus = pControl;
            s_pControlFocus->OnFocusIn();
            return true;
        }
    }

    // If we reached this point, the chain of dialogs didn't form a complete loop
    DXTRACE_ERR( "CDXUTDialog: Multiple dialogs are improperly chained together", E_FAIL );
    return false;
}
CDXUTControl* CDXUTDialog::GetNextControl( CDXUTControl* pControl )
{
    int index = pControl->m_Index + 1;

    CDXUTDialog* pDialog = pControl->m_pDialog;
    
    // Cycle through dialogs in the loop to find the next control. Note
    // that if only one control exists in all looped dialogs it will
    // be the returned 'next' control.
    while( index >= (int) pDialog->m_Controls.GetSize() )
    {
        pDialog = pDialog->m_pNextDialog;
        index = 0;
    }
    
    return pDialog->m_Controls.GetAt( index );    
}
CDXUTControl* CDXUTDialog::GetPrevControl( CDXUTControl* pControl )
{
    int index = pControl->m_Index - 1;

    CDXUTDialog* pDialog = pControl->m_pDialog;
    
    // Cycle through dialogs in the loop to find the next control. Note
    // that if only one control exists in all looped dialogs it will
    // be the returned 'previous' control.
    while( index < 0 )
    {
        pDialog = pDialog->m_pPrevDialog;
        if( pDialog == NULL )
            pDialog = pControl->m_pDialog;

        index = pDialog->m_Controls.GetSize() - 1;
    }
    
    return pDialog->m_Controls.GetAt( index );    
}
