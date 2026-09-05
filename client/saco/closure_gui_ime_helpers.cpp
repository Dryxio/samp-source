// Complete pinned IME source helpers.
#include "main.h"
extern CGame *pGame;
void CDXUTIMEEditBox::SendKey( BYTE nVirtKey )
{
    keybd_event( nVirtKey, 0, 0,               0 );
    keybd_event( nVirtKey, 0, KEYEVENTF_KEYUP, 0 );
}
HRESULT CDXUTIMEEditBox::StaticOnCreateDevice()
{
	_ImmDisableTextFrameService(-1);

    // Save the default input context
	s_hImcDef = _ImmGetContext( pGame->GetMainWindowHwnd() );
    _ImmReleaseContext( pGame->GetMainWindowHwnd(), s_hImcDef );

    return S_OK;
}
void CDXUTIMEEditBox::SendCompString()
{
    for( int i = 0; i < lstrlenW( s_CompString.GetBuffer() ); ++i )
        MsgProc( WM_CHAR, (WPARAM)s_CompString[i], 0 );
}
WCHAR& CUniBuffer::operator[]( int n )  // No param checking
{
    // This version of operator[] is called only
    // if we are asking for write access, so
    // re-analysis is required.
    m_bAnalyseRequired = true;
    return m_pwszBuffer[n];
}
