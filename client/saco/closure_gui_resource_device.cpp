#include "d3d9/common/dxstdafx.h"
CDXUTDialogResourceManager::CDXUTDialogResourceManager()
{
    m_pd3dDevice = NULL;
    m_pStateBlock = NULL;
    m_pSprite = NULL;
}
HRESULT CDXUTDialogResourceManager::OnCreateDevice( LPDIRECT3DDEVICE9 pd3dDevice )
{
    HRESULT hr = S_OK;
    int i=0;

    m_pd3dDevice = pd3dDevice;
    
    for( i=0; i < m_FontCache.GetSize(); i++ )
    {
        hr = CreateFont( i );
        if( FAILED(hr) )
            return hr;
    }
    
    for( i=0; i < m_TextureCache.GetSize(); i++ )
    {
        hr = CreateTexture( i );
        if( FAILED(hr) )
            return hr;
    }

    hr = D3DXCreateSprite( pd3dDevice, &m_pSprite );
    if( FAILED(hr) )
        return DXUT_ERR( "D3DXCreateSprite", hr );

    // Call CDXUTIMEEditBox's StaticOnCreateDevice()
    // to initialize certain window-dependent data.
    CDXUTIMEEditBox::StaticOnCreateDevice();

    return S_OK;
}
HRESULT CDXUTDialogResourceManager::OnResetDevice()
{
    HRESULT hr = S_OK;

    for( int i=0; i < m_FontCache.GetSize(); i++ )
    {
        DXUTFontNode* pFontNode = m_FontCache.GetAt( i );

        if( pFontNode->pFont )
            pFontNode->pFont->OnResetDevice();
    }

    if( m_pSprite )
        m_pSprite->OnResetDevice();

    m_pd3dDevice->CreateStateBlock( D3DSBT_ALL, &m_pStateBlock );

    return S_OK;
}
void CDXUTDialogResourceManager::OnLostDevice()
{
    for( int i=0; i < m_FontCache.GetSize(); i++ )
    {
        DXUTFontNode* pFontNode = m_FontCache.GetAt( i );

        if( pFontNode->pFont )
            pFontNode->pFont->OnLostDevice();
    }

    if( m_pSprite )
        m_pSprite->OnLostDevice();

    SAFE_RELEASE( m_pStateBlock  );
}
void CDXUTDialogResourceManager::OnDestroyDevice()
{
    int i=0; 

    //m_pd3dDevice = NULL;

    // Release the resources but don't clear the cache, as these will need to be
    // recreated if the device is recreated
    for( i=0; i < m_FontCache.GetSize(); i++ )
    {
        DXUTFontNode* pFontNode = m_FontCache.GetAt( i );
        SAFE_RELEASE( pFontNode->pFont );
    }
    
    for( i=0; i < m_TextureCache.GetSize(); i++ )
    {
        DXUTTextureNode* pTextureNode = m_TextureCache.GetAt( i );
        SAFE_RELEASE( pTextureNode->pTexture );
    }

    SAFE_RELEASE( m_pSprite );
}
