// Complete definitions extracted from pinned DXUTgui.cpp by tools/prepare_gui_edit.py.
#include "main.h"
#include <strsafe.h>
#define DXUT_MAX_EDITBOXLENGTH 0xFFFF
inline int RectWidth( RECT &rc ) { return ( (rc).right - (rc).left ); }
PCHAR ConvertWideToAnsi(PWCHAR wszText)
{
	static CHAR MultiByteStr[2048];
	ZeroMemory(&MultiByteStr[0], sizeof(MultiByteStr));

	int nRequiredSize = WideCharToMultiByte(CP_ACP, 0, wszText, wcslen(wszText), NULL, 0, NULL, NULL);
	if(nRequiredSize <= 2048)
	{
		WideCharToMultiByte(CP_ACP, 0, wszText, wcslen(wszText), MultiByteStr, nRequiredSize, NULL, NULL);
		MultiByteStr[nRequiredSize] = 0;
	}
	return MultiByteStr;
}

void ConvertAnsiToWide(LPCSTR wszSrc, LPWSTR wszDest, int nSize)
{
	memset(wszDest, 0, sizeof(WCHAR) * nSize);

	int nCount = MultiByteToWideChar(CP_ACP, 0, wszSrc, strlen(wszSrc), NULL, 0);
	if(nCount < nSize)
		MultiByteToWideChar(CP_ACP, 0, wszSrc, strlen(wszSrc), wszDest, nCount);
}

PCHAR CDXUTEditBox::GetTextA()
{
	return ConvertWideToAnsi((PWCHAR)m_Buffer.GetBuffer());
}

void CDXUTEditBox::SetText( LPCTSTR wszText, bool bSelected )
{
    assert( wszText != NULL );

    m_Buffer.SetText( wszText );
    m_nFirstVisible = 0;
    // Move the caret to the end of the text
    PlaceCaret( m_Buffer.GetTextSize() );
    m_nSelStart = bSelected ? 0 : m_nCaret;
}

void CDXUTEditBox::PlaceCaret( int nCP )
{
    assert( nCP >= 0 && nCP <= m_Buffer.GetTextSize() );
    m_nCaret = nCP;

    // Obtain the X offset of the character.
    int nX1st, nX, nX2;
    m_Buffer.CPtoX( m_nFirstVisible, FALSE, &nX1st );  // 1st visible char
    m_Buffer.CPtoX( nCP, FALSE, &nX );  // LEAD
    // If nCP is the NULL terminator, get the leading edge instead of trailing.
    if( nCP == m_Buffer.GetTextSize() )
        nX2 = nX;
    else
        m_Buffer.CPtoX( nCP, TRUE, &nX2 );  // TRAIL

    // If the left edge of the char is smaller than the left edge of the 1st visible char,
    // we need to scroll left until this char is visible.
    if( nX < nX1st )
    {
        // Simply make the first visible character the char at the new caret position.
        m_nFirstVisible = nCP;
    }
    else
    // If the right of the character is bigger than the offset of the control's
    // right edge, we need to scroll right to this character.
    if( nX2 > nX1st + RectWidth( m_rcText ) )
    {
        // Compute the X of the new left-most pixel
        int nXNewLeft = nX2 - RectWidth( m_rcText );

        // Compute the char position of this character
        int nCPNew1st, nNewTrail;
        m_Buffer.XtoCP( nXNewLeft, &nCPNew1st, &nNewTrail );

        // If this coordinate is not on a character border,
        // start from the next character so that the caret
        // position does not fall outside the text rectangle.
        int nXNew1st;
        m_Buffer.CPtoX( nCPNew1st, FALSE, &nXNew1st );
        if( nXNew1st < nXNewLeft )
            ++nCPNew1st;

        m_nFirstVisible = nCPNew1st;
    }
}

bool CUniBuffer::SetBufferSize( int nNewSize )
{
    // If the current size is already the maximum allowed,
    // we can't possibly allocate more.
    if( m_nBufferSize == DXUT_MAX_EDITBOXLENGTH )
        return false;

    int nAllocateSize = ( nNewSize == -1 || nNewSize < m_nBufferSize * 2 ) ? ( m_nBufferSize ? m_nBufferSize * 2 : 256 ) : nNewSize * 2;

    // Cap the buffer size at the maximum allowed.
    if( nAllocateSize > DXUT_MAX_EDITBOXLENGTH )
        nAllocateSize = DXUT_MAX_EDITBOXLENGTH;

    WCHAR *pTempBuffer = new WCHAR[nAllocateSize];
    if( !pTempBuffer )
        return false;
    if( m_pwszBuffer )
    {
        CopyMemory( pTempBuffer, m_pwszBuffer, m_nBufferSize * sizeof(WCHAR) );
        delete[] m_pwszBuffer;
    }
    else
    {
        ZeroMemory( pTempBuffer, sizeof(WCHAR) * nAllocateSize );
    }

    m_pwszBuffer = pTempBuffer;
    m_nBufferSize = nAllocateSize;
    return true;
}

bool CUniBuffer::SetText( LPCWSTR wszText )
{
    assert( wszText != NULL );

    int nRequired = int(lstrlenW( wszText ) + 1);

    // Check for maximum length allowed
    if( nRequired >= DXUT_MAX_EDITBOXLENGTH )
        return false;

    while( GetBufferSize() < nRequired )
        if( !SetBufferSize( -1 ) )
            break;
    // Check again in case out of memory occurred inside while loop.
    if( GetBufferSize() >= nRequired )
    {
        StringCchCopyW( m_pwszBuffer, GetBufferSize(), wszText );
        m_bAnalyseRequired = true;
        return true;
    }
    else
        return false;
}

bool CUniBuffer::SetText( LPCTSTR wszText )
{
	if(lstrlenA(wszText) > 1024) return false;

	WCHAR szBuffer[2049];
	ConvertAnsiToWide(wszText, szBuffer, 2048);
    SetText(szBuffer);
	return true;
}

HRESULT CUniBuffer::CPtoX( int nCP, BOOL bTrail, int *pX )
{
    assert( pX );
    *pX = 0;  // Default

    HRESULT hr = S_OK;
    if( m_bAnalyseRequired )
        hr = Analyse();

    if( SUCCEEDED( hr ) )
        hr = _ScriptStringCPtoX( m_Analysis, nCP, bTrail, pX );

    return hr;
}

HRESULT CUniBuffer::XtoCP( int nX, int *pCP, int *pnTrail )
{
    assert( pCP && pnTrail );
    *pCP = 0; *pnTrail = FALSE;  // Default

    HRESULT hr = S_OK;
    if( m_bAnalyseRequired )
        hr = Analyse();

    if( SUCCEEDED( hr ) )
        hr = _ScriptStringXtoCP( m_Analysis, nX, pCP, pnTrail );

    // If the coordinate falls outside the text region, we
    // can get character positions that don't exist.  We must
    // filter them here and convert them to those that do exist.
    if( *pCP == -1 && *pnTrail == TRUE )
    {
        *pCP = 0; *pnTrail = FALSE;
    } else
    if( *pCP > lstrlenW( m_pwszBuffer ) && *pnTrail == FALSE )
    {
        *pCP = lstrlenW( m_pwszBuffer ); *pnTrail = TRUE;
    }

    return hr;
}

WCHAR *MaskPasswordString(const WCHAR *text)
{
    static WCHAR masked[256];
    memset(masked, 0, 511);
    int length = (int)wcslen(text);
    int i = 0;
    if(length <= 255) {
        for(; i != length; i++) masked[i] = 0x25CF;
        masked[i] = 0;
    }
    return masked;
}

HRESULT CUniBuffer::Analyse()
{
    if( m_Analysis )
        _ScriptStringFree( &m_Analysis );

    SCRIPT_CONTROL ScriptControl; // For uniscribe
    SCRIPT_STATE   ScriptState;   // For uniscribe
    ZeroMemory( &ScriptControl, sizeof(ScriptControl) );
    ZeroMemory( &ScriptState, sizeof(ScriptState) );
    _ScriptApplyDigitSubstitution ( NULL, &ScriptControl, &ScriptState );

    if( !m_pFontNode )
        return E_FAIL;

    HRESULT hr;
    if(!field_12) {
        hr = _ScriptStringAnalyse( m_pFontNode->pFont ? m_pFontNode->pFont->GetDC() : NULL,
                                       m_pwszBuffer,
                                       lstrlenW( m_pwszBuffer ) + 1,  // NULL is also analyzed.
                                       lstrlenW( m_pwszBuffer ) * 3 / 2 + 16,
                                       -1,
                                       SSA_BREAK | SSA_GLYPHS | SSA_FALLBACK | SSA_LINK,
                                       0,
                                       &ScriptControl,
                                       &ScriptState,
                                       NULL,
                                       NULL,
                                       NULL,
                                       &m_Analysis );
    } else {
        hr = _ScriptStringAnalyse( m_pFontNode->pFont ? m_pFontNode->pFont->GetDC() : NULL,
                                       MaskPasswordString(m_pwszBuffer),
                                       lstrlenW( MaskPasswordString(m_pwszBuffer) ) + 1,  // NULL is also analyzed.
                                       lstrlenW( MaskPasswordString(m_pwszBuffer) ) * 3 / 2 + 16,
                                       -1,
                                       SSA_BREAK | SSA_GLYPHS | SSA_FALLBACK | SSA_LINK,
                                       0,
                                       &ScriptControl,
                                       &ScriptState,
                                       NULL,
                                       NULL,
                                       NULL,
                                       &m_Analysis );
    }
    if( SUCCEEDED( hr ) )
        m_bAnalyseRequired = false;  // Analysis is up-to-date
    return hr;
}
