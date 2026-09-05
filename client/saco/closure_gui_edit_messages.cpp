#include "d3d9/common/dxstdafx.h"
#define DXUT_MAX_EDITBOXLENGTH 0xFFFF
bool CDXUTEditBox::MsgProc( UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    if( !m_bEnabled || !m_bVisible )
        return false;

    switch( uMsg )
    {
	    case WM_CHAR:
        {
            switch( (WCHAR)wParam )
            {
                // Backspace
                case VK_BACK:
                {
                    // If there's a selection, treat this
                    // like a delete key.
                    if( m_nCaret != m_nSelStart )
                    {
                        DeleteSelectionText();
                        m_pDialog->SendEvent( EVENT_EDITBOX_CHANGE, true, this );
                    }
                    else
                    if( m_nCaret > 0 )
                    {
                        // Move the caret, then delete the char.
                        PlaceCaret( m_nCaret - 1 );
                        m_nSelStart = m_nCaret;
                        m_Buffer.RemoveChar( m_nCaret );
                        m_pDialog->SendEvent( EVENT_EDITBOX_CHANGE, true, this );
                    }
                    ResetCaretBlink();
                    break;
                }

                case 24:        // Ctrl-X Cut
                case VK_CANCEL: // Ctrl-C Copy
                {
                    CopyToClipboard();

                    // If the key is Ctrl-X, delete the selection too.
                    if( (WCHAR)wParam == 24 )
                    {
                        DeleteSelectionText();
                        m_pDialog->SendEvent( EVENT_EDITBOX_CHANGE, true, this );
                    }

                    break;
                }

                // Ctrl-V Paste
                case 22:
                {
                    PasteFromClipboard();
                    m_pDialog->SendEvent( EVENT_EDITBOX_CHANGE, true, this );
                    break;
                }

                // Ctrl-A Select All
                case 1:
                    if( m_nSelStart == m_nCaret )
                    {
                        m_nSelStart = 0;
                        PlaceCaret( m_Buffer.GetTextSize() );
                    }
                    break;

				// Enter char does nothing 
                case VK_RETURN:
                    break;

                // Junk characters we don't want in the string
                case 26:  // Ctrl Z
                case 2:   // Ctrl B
                case 14:  // Ctrl N
                case 19:  // Ctrl S
                case 4:   // Ctrl D
                case 6:   // Ctrl F
                case 7:   // Ctrl G
                case 10:  // Ctrl J
                case 11:  // Ctrl K
                case 12:  // Ctrl L
                case 17:  // Ctrl Q
                case 23:  // Ctrl W
                case 5:   // Ctrl E
                case 18:  // Ctrl R
                case 20:  // Ctrl T
                case 25:  // Ctrl Y
                case 21:  // Ctrl U
                case 9:   // Ctrl I
                case 15:  // Ctrl O
                case 16:  // Ctrl P
                case 27:  // Ctrl [
                case 29:  // Ctrl ]
                case 28:  // Ctrl \ 
                    break;

                default:
                {
                    // If there's a selection and the user
                    // starts to type, the selection should
                    // be deleted.
                    if( m_nCaret != m_nSelStart )
                        DeleteSelectionText();

					if(strlen(GetTextA()) >= 128) return true;

                    bool bPlaceCaret = false;
                    if( !m_bInsertMode && m_nCaret < m_Buffer.GetTextSize() )
                    {
                        if( (WCHAR)wParam > 255 )
                        {
                            m_Buffer[m_nCaret] = (WCHAR)wParam;
                            PlaceCaret( m_nCaret + 1 );
                            m_nSelStart = m_nCaret;
                        }
                        else
                            bPlaceCaret = m_Buffer.OverwriteChar(m_nCaret, (CHAR)wParam);
                    }
                    else
                    {
                        bPlaceCaret = (WCHAR)wParam > 255 ?
                            m_Buffer.InsertChar(m_nCaret, (WCHAR)wParam) :
                            m_Buffer.InsertChar(m_nCaret, (CHAR)wParam);
                    }
                    if( bPlaceCaret )
                    {
                            PlaceCaret( m_nCaret + 1 );
                            m_nSelStart = m_nCaret;
                    }
                    ResetCaretBlink();
                    m_pDialog->SendEvent( EVENT_EDITBOX_CHANGE, true, this );
                }
            }
            return true;
        }

		case WM_KEYUP:
		{
			if(wParam == VK_RETURN) {
                // Invoke the callback when the user presses Enter.
                m_pDialog->SendEvent( EVENT_EDITBOX_STRING, true, this );
				return true;
			}
			break;
		}
    }
    return false;
}
bool CUniBuffer::InsertChar( int nIndex, WCHAR tchr )
{
    assert( nIndex >= 0 );

    if( nIndex < 0 || nIndex > lstrlenW( m_pwszBuffer ) )
        return false;  // invalid index

    // Check for maximum length allowed
    if( GetTextSize() + 1 >= DXUT_MAX_EDITBOXLENGTH )
        return false;

    if( lstrlenW( m_pwszBuffer ) + 1 >= m_nBufferSize )
    {
        if( !SetBufferSize( -1 ) )
            return false;  // out of memory
    }

    assert( m_nBufferSize >= 2 );

    // Shift the characters after the index, start by copying the null terminator
    WCHAR* dest = m_pwszBuffer + lstrlenW(m_pwszBuffer)+1;
    WCHAR* stop = m_pwszBuffer + nIndex;
    WCHAR* src = dest - 1;

    while( dest > stop )
    {
        *dest-- = *src--;
    }

    // Set new character
    m_pwszBuffer[ nIndex ] = tchr;
    m_bAnalyseRequired = true;

    return true;
}
bool CUniBuffer::InsertChar(int nIndex, CHAR tchr)
{
	WCHAR WideCharStr[2];
	ZeroMemory(&WideCharStr[0], sizeof(WideCharStr));

	if(field_8)
	{
		CHAR MultiByteStr[3];
		MultiByteStr[0] = field_8;
		MultiByteStr[1] = tchr;
		MultiByteStr[2] = 0;
		MultiByteToWideChar(CP_ACP, 0, MultiByteStr, 2, WideCharStr, 1);
		InsertChar(nIndex, WideCharStr[0]);
		field_8 = 0;
		return true;
	}
	else if(IsDBCSLeadByteEx(CP_ACP, tchr))
	{
		field_8 = tchr;
		return false;
	}
	else
	{
		MultiByteToWideChar(CP_ACP, 0, &tchr, 1, WideCharStr, 1);
		InsertChar(nIndex, WideCharStr[0]);
		return true;
	}
}
