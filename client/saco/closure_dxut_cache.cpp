// Complete original resource-cache lifetime.
#include "d3d9/common/dxstdafx.h"
CDXUTResourceCache& DXUTGetGlobalResourceCache()
{
    // Using an accessor function gives control of the construction order
    static CDXUTResourceCache cache;
    return cache;
}
CDXUTResourceCache::~CDXUTResourceCache()
{
    OnDestroyDevice();

    m_TextureCache.RemoveAll();
    m_EffectCache.RemoveAll();
    m_FontCache.RemoveAll();
}
HRESULT CDXUTResourceCache::OnCreateDevice( IDirect3DDevice9 *pd3dDevice )
{
    return S_OK;
}
HRESULT CDXUTResourceCache::OnResetDevice( IDirect3DDevice9 *pd3dDevice )
{
    // Call OnResetDevice on all effect and font objects
    for( int i = 0; i < m_EffectCache.GetSize(); ++i )
        m_EffectCache[i].pEffect->OnResetDevice();
    for( int i = 0; i < m_FontCache.GetSize(); ++i )
        m_FontCache[i].pFont->OnResetDevice();


    return S_OK;
}
HRESULT CDXUTResourceCache::OnLostDevice()
{
    // Call OnLostDevice on all effect and font objects
    for( int i = 0; i < m_EffectCache.GetSize(); ++i )
        m_EffectCache[i].pEffect->OnLostDevice();
    for( int i = 0; i < m_FontCache.GetSize(); ++i )
        m_FontCache[i].pFont->OnLostDevice();

    // Release all the default pool textures
    for( int i = m_TextureCache.GetSize() - 1; i >= 0; --i )
        if( m_TextureCache[i].Pool == D3DPOOL_DEFAULT )
        {
            SAFE_RELEASE( m_TextureCache[i].pTexture );
            m_TextureCache.Remove( i );  // Remove the entry
        }

    return S_OK;
}
HRESULT CDXUTResourceCache::OnDestroyDevice()
{
    // Release all resources
    for( int i = m_EffectCache.GetSize() - 1; i >= 0; --i )
    {
        SAFE_RELEASE( m_EffectCache[i].pEffect );
        m_EffectCache.Remove( i );
    }
    for( int i = m_FontCache.GetSize() - 1; i >= 0; --i )
    {
        SAFE_RELEASE( m_FontCache[i].pFont );
        m_FontCache.Remove( i );
    }
    for( int i = m_TextureCache.GetSize() - 1; i >= 0; --i )
    {
        SAFE_RELEASE( m_TextureCache[i].pTexture );
        m_TextureCache.Remove( i );
    }

    return S_OK;
}
