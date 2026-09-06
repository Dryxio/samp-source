#include "main.h"
extern CGame *pGame;
#define LANG_CHT MAKELANGID(LANG_CHINESE,SUBLANG_CHINESE_TRADITIONAL)
void CDXUTEditBox::OnFocusIn()
{
    CDXUTControl::OnFocusIn();

    ResetCaretBlink();
}

void CDXUTIMEEditBox::FinalizeString( bool bSend )
{
    HIMC hImc;
	if( NULL == ( hImc = _ImmGetContext( pGame->GetMainWindowHwnd() ) ) )
        return;

    static bool bProcessing = false;
    if( bProcessing )    // avoid infinite recursion
    {
        _ImmReleaseContext( pGame->GetMainWindowHwnd(), hImc );
        return;
    }
    bProcessing = true;

    if( !s_bInsertOnType && bSend )
    {
        // Send composition string to app.
        LONG lLength = lstrlenW( s_CompString.GetBuffer() );
        // In case of CHT IME, don't send the trailing double byte space, if it exists.
        if( GetLanguage() == LANG_CHT
            && s_CompString[lLength - 1] == 0x3000 )
        {
            s_CompString[lLength - 1] = 0;
        }
        SendCompString();
    }

    ResetCompositionString();
    // Clear composition string in IME
    _ImmNotifyIME( hImc, NI_COMPOSITIONSTR, CPS_CANCEL, 0 );
    // the following line is necessary as Korean IME doesn't close cand list
    // when comp string is cancelled.
    _ImmNotifyIME( hImc, NI_CLOSECANDIDATE, 0, 0 ); 
    _ImmReleaseContext( pGame->GetMainWindowHwnd(), hImc );
    bProcessing = false;
}

void CDXUTIMEEditBox::OnFocusIn()
{
    CDXUTEditBox::OnFocusIn();

    if(!s_bEnableImeSystem) return;
    {
        _ImmAssociateContext( pGame->GetMainWindowHwnd(), s_hImcDef );
        CheckToggleState();
    }

    //
    // Set up the IME global state according to the current instance state
    //
    HIMC hImc;
    if( NULL != ( hImc = _ImmGetContext( pGame->GetMainWindowHwnd() ) ) ) 
    {
        if( !s_bEnableImeSystem )
            s_ImeState = IMEUI_STATE_OFF;

        _ImmReleaseContext( pGame->GetMainWindowHwnd(), hImc );
        CheckToggleState();
    }
}

void CDXUTIMEEditBox::OnFocusOut()
{
    CDXUTEditBox::OnFocusOut();

    if(!s_bEnableImeSystem) return;
    FinalizeString( false );  // Don't send the comp string as to match RichEdit behavior

    _ImmAssociateContext( pGame->GetMainWindowHwnd(), NULL );
}
