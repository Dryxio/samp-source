// Complete pinned DXUT drawing definitions; no original machine-code bytes.
#include "d3d9/common/dxstdafx.h"
#include <new>
#include "d3dhook/ID3DXFontHook.h"
#undef min
#undef max
struct DXUT_SCREEN_VERTEX
{
    float x, y, z, h;
    D3DCOLOR color;
    float tu, tv;

    static DWORD FVF;
};
DWORD DXUT_SCREEN_VERTEX::FVF = D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1;
inline int RectWidth( RECT &rc ) { return ( (rc).right - (rc).left ); }
inline int RectHeight( RECT &rc ) { return ( (rc).bottom - (rc).top ); }
DXUTTextureNode* CDXUTDialog::GetTexture( UINT index )
{
    if( NULL == m_pManager )
        return NULL;
    return m_pManager->GetTextureNode( m_Textures.GetAt( index ) );
}
HRESULT CDXUTDialog::DrawRect( RECT* pRect, D3DCOLOR color )
{
    RECT rcScreen = *pRect;
    OffsetRect( &rcScreen, m_x, m_y );

    // If caption is enabled, offset the Y position by its height.
    if( m_bCaption )
        OffsetRect( &rcScreen, 0, m_nCaptionHeight );

    DXUT_SCREEN_VERTEX vertices[4] =
    {
        (float) rcScreen.left -0.5f,  (float) rcScreen.top -0.5f,    0.5f, 1.0f, color, 0, 0,
        (float) rcScreen.right -0.5f, (float) rcScreen.top -0.5f,    0.5f, 1.0f, color, 0, 0, 
        (float) rcScreen.right -0.5f, (float) rcScreen.bottom -0.5f, 0.5f, 1.0f, color, 0, 0, 
        (float) rcScreen.left -0.5f,  (float) rcScreen.bottom -0.5f, 0.5f, 1.0f, color, 0, 0,
    };

    IDirect3DDevice9* pd3dDevice = m_pManager->GetD3DDevice();

    // Since we're doing our own drawing here we need to flush the sprites
    m_pManager->m_pSprite->Flush();
    IDirect3DVertexDeclaration9 *pDecl = NULL;
    pd3dDevice->GetVertexDeclaration( &pDecl );  // Preserve the sprite's current vertex decl
    pd3dDevice->SetFVF( DXUT_SCREEN_VERTEX::FVF );

    pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_SELECTARG2 );
    pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG2 );

    pd3dDevice->DrawPrimitiveUP( D3DPT_TRIANGLEFAN, 2, vertices, sizeof(DXUT_SCREEN_VERTEX) );

    pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE );
    pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_MODULATE );

    // Restore the vertex decl
    pd3dDevice->SetVertexDeclaration( pDecl );
    pDecl->Release();

    return S_OK;
}
HRESULT CDXUTDialog::DrawSprite( CDXUTElement* pElement, RECT* prcDest )
{
    // No need to draw fully transparent layers
    if( pElement->TextureColor.Current.a == 0 )
        return S_OK;

    RECT rcTexture = pElement->rcTexture;
    
    RECT rcScreen = *prcDest;
    OffsetRect( &rcScreen, m_x, m_y );

    // If caption is enabled, offset the Y position by its height.
    if( m_bCaption )
        OffsetRect( &rcScreen, 0, m_nCaptionHeight );

    DXUTTextureNode* pTextureNode = GetTexture( pElement->iTexture );
    if( pTextureNode == NULL )
        return E_FAIL;
    
    float fScaleX = (float) RectWidth( rcScreen ) / RectWidth( rcTexture );
    float fScaleY = (float) RectHeight( rcScreen ) / RectHeight( rcTexture );

    D3DXMATRIXA16 matTransform;
    D3DXMatrixScaling( &matTransform, fScaleX, fScaleY, 1.0f );

    m_pManager->m_pSprite->SetTransform( &matTransform );
    
    D3DXVECTOR3 vPos( (float)rcScreen.left, (float)rcScreen.top, 0.0f );

    vPos.x /= fScaleX;
    vPos.y /= fScaleY;

    return m_pManager->m_pSprite->Draw( pTextureNode->pTexture, &rcTexture, NULL, &vPos, pElement->TextureColor.Current );
}
HRESULT CDXUTDialog::DrawPolyLine( POINT* apPoints, UINT nNumPoints, D3DCOLOR color )
{
    DXUT_SCREEN_VERTEX* vertices = new DXUT_SCREEN_VERTEX[ nNumPoints ];
    if( vertices == NULL )
        return E_OUTOFMEMORY;

    DXUT_SCREEN_VERTEX* pVertex = vertices;
    POINT* pt = apPoints;
    for( UINT i=0; i < nNumPoints; i++ )
    {
        pVertex->x = m_x + (float) pt->x;
        pVertex->y = m_y + (float) pt->y;
        pVertex->z = 0.5f;
        pVertex->h = 1.0f;
        pVertex->color = color;
        pVertex->tu = 0.0f;
        pVertex->tv = 0.0f;

        pVertex++;
        pt++;
    }

    IDirect3DDevice9* pd3dDevice = m_pManager->GetD3DDevice();

    // Since we're doing our own drawing here we need to flush the sprites
    m_pManager->m_pSprite->Flush();
    IDirect3DVertexDeclaration9 *pDecl = NULL;
    pd3dDevice->GetVertexDeclaration( &pDecl );  // Preserve the sprite's current vertex decl
    pd3dDevice->SetFVF( DXUT_SCREEN_VERTEX::FVF );

    pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_SELECTARG2 );
    pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG2 );

    pd3dDevice->DrawPrimitiveUP( D3DPT_LINESTRIP, nNumPoints - 1, vertices, sizeof(DXUT_SCREEN_VERTEX) );

    pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE );
    pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_MODULATE );

    // Restore the vertex decl
    pd3dDevice->SetVertexDeclaration( pDecl );
    pDecl->Release();

    SAFE_DELETE_ARRAY( vertices );
    return S_OK;
}
HRESULT CDXUTDialog::DrawText( LPCTSTR strText, CDXUTElement* pElement, RECT* prcDest, bool bShadow, int nCount )
{
    HRESULT hr = S_OK;

    // No need to draw fully transparent layers
    if( pElement->FontColor.Current.a == 0 )
        return S_OK;

    RECT rcScreen = *prcDest;
    OffsetRect( &rcScreen, m_x, m_y );

    // If caption is enabled, offset the Y position by its height.
    if( m_bCaption )
        OffsetRect( &rcScreen, 0, m_nCaptionHeight );

    //debug
    //DrawRect( &rcScreen, D3DCOLOR_ARGB(100, 255, 0, 0) );

    D3DXMATRIXA16 matTransform;
    D3DXMatrixIdentity( &matTransform );
    m_pManager->m_pSprite->SetTransform( &matTransform );

    DXUTFontNode* pFontNode = GetFont( pElement->iFont );
    
    if( bShadow )
    {
        RECT rcShadow = rcScreen;
        OffsetRect( &rcShadow, 1, 1 );
		if(pFontNode->pFont) {
			hr = pFontNode->pFont->DrawText( m_pManager->m_pSprite, strText, nCount, &rcShadow, pElement->dwTextFormat, D3DCOLOR_ARGB(DWORD(pElement->FontColor.Current.a * 255), 0, 0, 0) );
		} else {
			m_pManager->CreateFont(pElement->iFont);
		}

		if( FAILED(hr) )
            return hr;
    }

	if(pFontNode->pFont) {
		hr = pFontNode->pFont->DrawText( m_pManager->m_pSprite, strText, nCount, &rcScreen, pElement->dwTextFormat, pElement->FontColor.Current );
	} else {
		m_pManager->CreateFont(pElement->iFont);
	}

	if( FAILED(hr) )
        return hr;

    return S_OK;
}
HRESULT CDXUTDialog::DrawText( LPCWSTR strText, CDXUTElement* pElement, RECT* prcDest, bool bShadow, int nCount )
{
    HRESULT hr = S_OK;

    // No need to draw fully transparent layers
    if( pElement->FontColor.Current.a == 0 )
        return S_OK;

    RECT rcScreen = *prcDest;
    OffsetRect( &rcScreen, m_x, m_y );

    // If caption is enabled, offset the Y position by its height.
    if( m_bCaption )
        OffsetRect( &rcScreen, 0, m_nCaptionHeight );

    //debug
    //DrawRect( &rcScreen, D3DCOLOR_ARGB(100, 255, 0, 0) );

    D3DXMATRIXA16 matTransform;
    D3DXMatrixIdentity( &matTransform );
    m_pManager->m_pSprite->SetTransform( &matTransform );

    DXUTFontNode* pFontNode = GetFont( pElement->iFont );
    
    if( bShadow )
    {
        RECT rcShadow = rcScreen;
        OffsetRect( &rcShadow, 1, 1 );
		if(pFontNode->pFont) {
			hr = pFontNode->pFont->DrawTextW( m_pManager->m_pSprite, strText, nCount, &rcShadow, pElement->dwTextFormat, D3DCOLOR_ARGB(DWORD(pElement->FontColor.Current.a * 255), 0, 0, 0) );
		} else {
			m_pManager->CreateFont(pElement->iFont);
		}

		if( FAILED(hr) )
            return hr;
    }

	if(pFontNode->pFont) {
		hr = pFontNode->pFont->DrawTextW( m_pManager->m_pSprite, strText, nCount, &rcScreen, pElement->dwTextFormat, pElement->FontColor.Current );
	} else {
		m_pManager->CreateFont(pElement->iFont);
	}

	if( FAILED(hr) )
        return hr;

    return S_OK;
}
HRESULT CDXUTDialog::CalcTextRect( LPCTSTR strText, CDXUTElement* pElement, RECT* prcDest, int nCount )
{
    HRESULT hr = S_OK;

    DXUTFontNode* pFontNode = GetFont( pElement->iFont );
    if( pFontNode == NULL )
        return E_FAIL;

    DWORD dwTextFormat = pElement->dwTextFormat | DT_CALCRECT;
    // Since we are only computing the rectangle, we don't need a sprite.
    hr = pFontNode->pFont->DrawText( NULL, strText, nCount, prcDest, dwTextFormat, pElement->FontColor.Current );
    if( FAILED(hr) )
        return hr;

    return S_OK;
}
HRESULT CDXUTDialog::CalcTextRect( LPCWSTR strText, CDXUTElement* pElement, RECT* prcDest, int nCount )
{
    HRESULT hr = S_OK;

    DXUTFontNode* pFontNode = GetFont( pElement->iFont );
    if( pFontNode == NULL )
        return E_FAIL;

    DWORD dwTextFormat = pElement->dwTextFormat | DT_CALCRECT;
    // Since we are only computing the rectangle, we don't need a sprite.
    hr = pFontNode->pFont->DrawTextW( NULL, strText, nCount, prcDest, dwTextFormat, pElement->FontColor.Current );
    if( FAILED(hr) )
        return hr;

    return S_OK;
}
HRESULT CDXUTDialogResourceManager::CreateFont( UINT iFont )
{
    HRESULT hr = S_OK;

    DXUTFontNode* pFontNode = m_FontCache.GetAt( iFont );

    SAFE_RELEASE( pFontNode->pFont );
    
    D3DXCreateFont( m_pd3dDevice, pFontNode->nHeight, 0, pFontNode->nWeight, 1, FALSE, DEFAULT_CHARSET, 
                    OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, 
                     pFontNode->strFace, &pFontNode->pFont );

	ID3DXFontHook *pFontHook = new ID3DXFontHook;
	pFontHook->m_pD3DFont = pFontNode->pFont;
	pFontNode->pFont = (ID3DXFont *)pFontHook;

	char szDebug[260];
	sprintf(szDebug,"CDXUTDialogResourceManager::CreateFont(pFont=0x%X,pD3DDevice=0x%X)",pFontNode->pFont,m_pd3dDevice);
	//OutputDebugString(szDebug);

    return S_OK;
}
HRESULT CDXUTDialog::OnRender( float fElapsedTime )
{   
    // If this assert triggers, you need to call CDXUTDialogResourceManager::On*Device() from inside
    // the application's device callbacks.  See the SDK samples for an example of how to do this.
    assert( m_pManager->GetD3DDevice() && m_pManager->m_pStateBlock && "To fix hook up CDXUTDialogResourceManager to device callbacks.  See comments for details" );

    // See if the dialog needs to be refreshed
    if( m_fTimeLastRefresh < s_fTimeRefresh )
    {
        m_fTimeLastRefresh = DXUTGetTime();
        Refresh();
    }

    // For invisible dialog, out now.
    if( !m_bVisible )
        return S_OK;

    DXUT_SCREEN_VERTEX vertices[4] =
    {
        (float)m_x,           (float)m_y,            0.5f, 1.0f, m_colorTopLeft, 0.0f, 0.5f, 
        (float)m_x + m_width, (float)m_y,            0.5f, 1.0f, m_colorTopRight, 1.0f, 0.5f,
        (float)m_x + m_width, (float)m_y + m_height, 0.5f, 1.0f, m_colorBottomRight, 1.0f, 1.0f, 
        (float)m_x,           (float)m_y + m_height, 0.5f, 1.0f, m_colorBottomLeft, 0.0f, 1.0f, 
    };

    IDirect3DDevice9* pd3dDevice = m_pManager->GetD3DDevice();     

    // Set up a state block here and restore it when finished drawing all the controls
    m_pManager->m_pStateBlock->Capture();

	m_pManager->m_pSprite->Begin( D3DXSPRITE_DONOTSAVESTATE );

    pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
    pd3dDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
    pd3dDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
    pd3dDevice->SetRenderState( D3DRS_ALPHATESTENABLE, FALSE );

    pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_SELECTARG2 );
    pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );

    pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1 );
    pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );

    pd3dDevice->SetVertexShader( NULL );
    pd3dDevice->SetPixelShader( NULL );

    //pd3dDevice->Clear( 0, NULL, D3DCLEAR_ZBUFFER, 0, 1.0f, 0 );
    pd3dDevice->SetRenderState( D3DRS_ZENABLE, FALSE );

    if( !m_bMinimized )
    {
        pd3dDevice->SetFVF( DXUT_SCREEN_VERTEX::FVF );
        pd3dDevice->DrawPrimitiveUP( D3DPT_TRIANGLEFAN, 2, vertices, sizeof(DXUT_SCREEN_VERTEX) );
    }


    pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE );
    pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
    pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
    
    pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_MODULATE );
    pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
    pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );

    pd3dDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );

    DXUTTextureNode* pTextureNode = GetTexture( 0 );
    pd3dDevice->SetTexture( 0, pTextureNode->pTexture );

    // Render the caption if it's enabled.
    if( m_bCaption )
    {
        // DrawSprite will offset the rect down by
        // m_nCaptionHeight, so adjust the rect higher
        // here to negate the effect.
        RECT rc = { 0, -m_nCaptionHeight, m_width, 0 };
        DrawSprite( &m_CapElement, &rc );
        rc.left += 5; // Make a left margin
        TCHAR wszOutput[256];
        StringCchCopy( wszOutput, 256, m_wszCaption );
        /*
		if( m_bMinimized )
            StringCchCat( wszOutput, 256, " (Minimized)" );*/
        DrawText( wszOutput, &m_CapElement, &rc, false );
    }

    // If the dialog is minimized, skip rendering
    // its controls.
    if( !m_bMinimized )
    {
        for( int i=0; i < m_Controls.GetSize(); i++ )
        {
            CDXUTControl* pControl = m_Controls.GetAt(i);   

            // Focused control is drawn last
            if( pControl == s_pControlFocus )
                continue;

            pControl->Render( pd3dDevice, fElapsedTime );
        }

        if( s_pControlFocus != NULL && s_pControlFocus->m_pDialog == this )
            s_pControlFocus->Render( pd3dDevice, fElapsedTime );
    }

    m_pManager->m_pSprite->End();

    m_pManager->m_pStateBlock->Apply();

    return S_OK;
}
