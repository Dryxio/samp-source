// Complete original SDK text helper.
#include "d3d9/common/dxstdafx.h"
CDXUTTextHelper::CDXUTTextHelper( ID3DXFont* pFont, ID3DXSprite* pSprite, int nLineHeight )
{
    m_pFont = pFont;
    m_pSprite = pSprite;
    m_clr = D3DXCOLOR(1,1,1,1);
    m_pt.x = 0; 
    m_pt.y = 0; 
    m_nLineHeight = nLineHeight;
}


//--------------------------------------------------------------------------------------
HRESULT CDXUTTextHelper::DrawFormattedTextLine( const TCHAR* strMsg, ... )
{
    TCHAR strBuffer[512];
    
    va_list args;
    va_start(args, strMsg);
    StringCchVPrintf( strBuffer, 512, strMsg, args );
    strBuffer[511] = L'\0';
    va_end(args);

    return DrawTextLine( strBuffer );
}


//--------------------------------------------------------------------------------------
HRESULT CDXUTTextHelper::DrawTextLine( const TCHAR* strMsg )
{
    if( NULL == m_pFont ) 
        return DXUT_ERR_MSGBOX( "DrawTextLine", E_INVALIDARG );

    HRESULT hr;
    RECT rc;
    SetRect( &rc, m_pt.x, m_pt.y, 0, 0 ); 
    hr = m_pFont->DrawText( m_pSprite, strMsg, -1, &rc, DT_NOCLIP, m_clr );
    if( FAILED(hr) )
        return DXTRACE_ERR_MSGBOX( "DrawText", hr );

    m_pt.y += m_nLineHeight;

    return S_OK;
}


HRESULT CDXUTTextHelper::DrawFormattedTextLine( RECT &rc, DWORD dwFlags, const TCHAR* strMsg, ... )
{
    TCHAR strBuffer[512];
    
    va_list args;
    va_start(args, strMsg);
    StringCchVPrintf( strBuffer, 512, strMsg, args );
    strBuffer[511] = L'\0';
    va_end(args);

    return DrawTextLine( rc, dwFlags, strBuffer );
}


HRESULT CDXUTTextHelper::DrawTextLine( RECT &rc, DWORD dwFlags, const TCHAR* strMsg )
{
    if( NULL == m_pFont ) 
        return DXUT_ERR_MSGBOX( "DrawTextLine", E_INVALIDARG );

    HRESULT hr;
    hr = m_pFont->DrawText( m_pSprite, strMsg, -1, &rc, dwFlags, m_clr );
    if( FAILED(hr) )
        return DXTRACE_ERR_MSGBOX( "DrawText", hr );

    m_pt.y += m_nLineHeight;

    return S_OK;
}


//--------------------------------------------------------------------------------------
void CDXUTTextHelper::Begin()
{
    if( m_pSprite )
        m_pSprite->Begin( D3DXSPRITE_ALPHABLEND | D3DXSPRITE_SORT_TEXTURE );
}
void CDXUTTextHelper::End()
{
    if( m_pSprite )
        m_pSprite->End();
}


//--------------------------------------------------------------------------------------
