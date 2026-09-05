// Complete pinned resource creation methods.
#include "d3d9/common/dxstdafx.h"
#include <strsafe.h>
HRESULT CDXUTResourceCache::CreateTextureFromFile( LPDIRECT3DDEVICE9 pDevice, LPCTSTR pSrcFile, LPDIRECT3DTEXTURE9 *ppTexture )
{
    return CreateTextureFromFileEx( pDevice, pSrcFile, D3DX_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT,
                                    0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED, D3DX_DEFAULT, D3DX_DEFAULT,
                                    0, NULL, NULL, ppTexture );
}
HRESULT CDXUTResourceCache::CreateTextureFromFileEx( LPDIRECT3DDEVICE9 pDevice, LPCTSTR pSrcFile, UINT Width, UINT Height, UINT MipLevels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, DWORD Filter, DWORD MipFilter, D3DCOLOR ColorKey, D3DXIMAGE_INFO *pSrcInfo, PALETTEENTRY *pPalette, LPDIRECT3DTEXTURE9 *ppTexture )
{
    // Search the cache for a matching entry.
    for( int i = 0; i < m_TextureCache.GetSize(); ++i )
    {
        DXUTCache_Texture &Entry = m_TextureCache[i];
        if( Entry.Location == DXUTCACHE_LOCATION_FILE &&
            !lstrcmpA( Entry.wszSource, pSrcFile ) &&
            Entry.Width == Width &&
            Entry.Height == Height &&
            Entry.MipLevels == MipLevels &&
            Entry.Usage == Usage &&
            Entry.Format == Format &&
            Entry.Pool == Pool &&
            Entry.Type == D3DRTYPE_TEXTURE )
        {
            // A match is found. Obtain the IDirect3DTexture9 interface and return that.
            return Entry.pTexture->QueryInterface( IID_IDirect3DTexture9, (LPVOID*)ppTexture );
        }
    }

    HRESULT hr;

    // No matching entry.  Load the resource and create a new entry.
    hr = D3DXCreateTextureFromFileEx( pDevice, pSrcFile, Width, Height, MipLevels, Usage, Format,
                                      Pool, Filter, MipFilter, ColorKey, pSrcInfo, pPalette, ppTexture );
    if( FAILED( hr ) )
        return hr;

    DXUTCache_Texture NewEntry;
    NewEntry.Location = DXUTCACHE_LOCATION_FILE;
    StringCchCopy( NewEntry.wszSource, MAX_PATH, pSrcFile );
    NewEntry.Width = Width;
    NewEntry.Height = Height;
    NewEntry.MipLevels = MipLevels;
    NewEntry.Usage = Usage;
    NewEntry.Format = Format;
    NewEntry.Pool = Pool;
    NewEntry.Type = D3DRTYPE_TEXTURE;
    (*ppTexture)->QueryInterface( IID_IDirect3DBaseTexture9, (LPVOID*)&NewEntry.pTexture );

    m_TextureCache.Add( NewEntry );
    return S_OK;
}
HRESULT CDXUTResourceCache::CreateTextureFromResource( LPDIRECT3DDEVICE9 pDevice, HMODULE hSrcModule, LPCTSTR pSrcResource, LPDIRECT3DTEXTURE9 *ppTexture )
{
    return CreateTextureFromResourceEx( pDevice, hSrcModule, pSrcResource, D3DX_DEFAULT, D3DX_DEFAULT,
                                        D3DX_DEFAULT, 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED, D3DX_DEFAULT,
                                        D3DX_DEFAULT, 0, NULL, NULL, ppTexture );
}
HRESULT CDXUTResourceCache::CreateTextureFromResourceEx( LPDIRECT3DDEVICE9 pDevice, HMODULE hSrcModule, LPCTSTR pSrcResource, UINT Width, UINT Height, UINT MipLevels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, DWORD Filter, DWORD MipFilter, D3DCOLOR ColorKey, D3DXIMAGE_INFO *pSrcInfo, PALETTEENTRY *pPalette, LPDIRECT3DTEXTURE9 *ppTexture )
{
    // Search the cache for a matching entry.
    for( int i = 0; i < m_TextureCache.GetSize(); ++i )
    {
        DXUTCache_Texture &Entry = m_TextureCache[i];
        if( Entry.Location == DXUTCACHE_LOCATION_RESOURCE &&
            Entry.hSrcModule == hSrcModule &&
            !lstrcmpA( Entry.wszSource, pSrcResource ) &&
            Entry.Width == Width &&
            Entry.Height == Height &&
            Entry.MipLevels == MipLevels &&
            Entry.Usage == Usage &&
            Entry.Format == Format &&
            Entry.Pool == Pool &&
            Entry.Type == D3DRTYPE_TEXTURE )
        {
            // A match is found. Obtain the IDirect3DTexture9 interface and return that.
            return Entry.pTexture->QueryInterface( IID_IDirect3DTexture9, (LPVOID*)ppTexture );
        }
    }

    HRESULT hr;

    // No matching entry.  Load the resource and create a new entry.
    hr = D3DXCreateTextureFromResourceEx( pDevice, hSrcModule, pSrcResource, Width, Height, MipLevels, Usage,
                                          Format, Pool, Filter, MipFilter, ColorKey, pSrcInfo, pPalette, ppTexture );
    if( FAILED( hr ) )
        return hr;

    DXUTCache_Texture NewEntry;
    NewEntry.Location = DXUTCACHE_LOCATION_RESOURCE;
    NewEntry.hSrcModule = hSrcModule;
    StringCchCopy( NewEntry.wszSource, MAX_PATH, pSrcResource );
    NewEntry.Width = Width;
    NewEntry.Height = Height;
    NewEntry.MipLevels = MipLevels;
    NewEntry.Usage = Usage;
    NewEntry.Format = Format;
    NewEntry.Pool = Pool;
    NewEntry.Type = D3DRTYPE_TEXTURE;
    (*ppTexture)->QueryInterface( IID_IDirect3DBaseTexture9, (LPVOID*)&NewEntry.pTexture );

    m_TextureCache.Add( NewEntry );
    return S_OK;
}
HRESULT CDXUTResourceCache::CreateCubeTextureFromFile( LPDIRECT3DDEVICE9 pDevice, LPCTSTR pSrcFile, LPDIRECT3DCUBETEXTURE9 *ppCubeTexture )
{
    return CreateCubeTextureFromFileEx( pDevice, pSrcFile, D3DX_DEFAULT, D3DX_DEFAULT, 0,
                                        D3DFMT_UNKNOWN, D3DPOOL_MANAGED, D3DX_DEFAULT, D3DX_DEFAULT,
                                        0, NULL, NULL, ppCubeTexture );
}
HRESULT CDXUTResourceCache::CreateCubeTextureFromFileEx( LPDIRECT3DDEVICE9 pDevice, LPCTSTR pSrcFile, UINT Size, UINT MipLevels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, DWORD Filter, DWORD MipFilter, D3DCOLOR ColorKey, D3DXIMAGE_INFO *pSrcInfo, PALETTEENTRY *pPalette, LPDIRECT3DCUBETEXTURE9 *ppCubeTexture )
{
    // Search the cache for a matching entry.
    for( int i = 0; i < m_TextureCache.GetSize(); ++i )
    {
        DXUTCache_Texture &Entry = m_TextureCache[i];
        if( Entry.Location == DXUTCACHE_LOCATION_FILE &&
            !lstrcmpA( Entry.wszSource, pSrcFile ) &&
            Entry.Width == Size &&
            Entry.MipLevels == MipLevels &&
            Entry.Usage == Usage &&
            Entry.Format == Format &&
            Entry.Pool == Pool &&
            Entry.Type == D3DRTYPE_CUBETEXTURE )
        {
            // A match is found. Obtain the IDirect3DCubeTexture9 interface and return that.
            return Entry.pTexture->QueryInterface( IID_IDirect3DCubeTexture9, (LPVOID*)ppCubeTexture );
        }
    }

    HRESULT hr;

    // No matching entry.  Load the resource and create a new entry.
    hr = D3DXCreateCubeTextureFromFileEx( pDevice, pSrcFile, Size, MipLevels, Usage, Format, Pool, Filter,
                                          MipFilter, ColorKey, pSrcInfo, pPalette, ppCubeTexture );
    if( FAILED( hr ) )
        return hr;

    DXUTCache_Texture NewEntry;
    NewEntry.Location = DXUTCACHE_LOCATION_FILE;
    StringCchCopy( NewEntry.wszSource, MAX_PATH, pSrcFile );
    NewEntry.Width = Size;
    NewEntry.MipLevels = MipLevels;
    NewEntry.Usage = Usage;
    NewEntry.Format = Format;
    NewEntry.Pool = Pool;
    NewEntry.Type = D3DRTYPE_CUBETEXTURE;
    (*ppCubeTexture)->QueryInterface( IID_IDirect3DBaseTexture9, (LPVOID*)&NewEntry.pTexture );

    m_TextureCache.Add( NewEntry );
    return S_OK;
}
HRESULT CDXUTResourceCache::CreateCubeTextureFromResource( LPDIRECT3DDEVICE9 pDevice, HMODULE hSrcModule, LPCTSTR pSrcResource, LPDIRECT3DCUBETEXTURE9 *ppCubeTexture )
{
    return CreateCubeTextureFromResourceEx( pDevice, hSrcModule, pSrcResource, D3DX_DEFAULT, D3DX_DEFAULT,
                                            0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED, D3DX_DEFAULT, D3DX_DEFAULT,
                                            0, NULL, NULL, ppCubeTexture );
}
HRESULT CDXUTResourceCache::CreateCubeTextureFromResourceEx( LPDIRECT3DDEVICE9 pDevice, HMODULE hSrcModule, LPCTSTR pSrcResource, UINT Size, UINT MipLevels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, DWORD Filter, DWORD MipFilter, D3DCOLOR ColorKey, D3DXIMAGE_INFO *pSrcInfo, PALETTEENTRY *pPalette, LPDIRECT3DCUBETEXTURE9 *ppCubeTexture )
{
    // Search the cache for a matching entry.
    for( int i = 0; i < m_TextureCache.GetSize(); ++i )
    {
        DXUTCache_Texture &Entry = m_TextureCache[i];
        if( Entry.Location == DXUTCACHE_LOCATION_RESOURCE &&
            Entry.hSrcModule == hSrcModule &&
            !lstrcmpA( Entry.wszSource, pSrcResource ) &&
            Entry.Width == Size &&
            Entry.MipLevels == MipLevels &&
            Entry.Usage == Usage &&
            Entry.Format == Format &&
            Entry.Pool == Pool &&
            Entry.Type == D3DRTYPE_CUBETEXTURE )
        {
            // A match is found. Obtain the IDirect3DCubeTexture9 interface and return that.
            return Entry.pTexture->QueryInterface( IID_IDirect3DCubeTexture9, (LPVOID*)ppCubeTexture );
        }
    }

    HRESULT hr;

    // No matching entry.  Load the resource and create a new entry.
    hr = D3DXCreateCubeTextureFromResourceEx( pDevice, hSrcModule, pSrcResource, Size, MipLevels, Usage, Format,
                                              Pool, Filter, MipFilter, ColorKey, pSrcInfo, pPalette, ppCubeTexture );
    if( FAILED( hr ) )
        return hr;

    DXUTCache_Texture NewEntry;
    NewEntry.Location = DXUTCACHE_LOCATION_RESOURCE;
    NewEntry.hSrcModule = hSrcModule;
    StringCchCopy( NewEntry.wszSource, MAX_PATH, pSrcResource );
    NewEntry.Width = Size;
    NewEntry.MipLevels = MipLevels;
    NewEntry.Usage = Usage;
    NewEntry.Format = Format;
    NewEntry.Pool = Pool;
    NewEntry.Type = D3DRTYPE_CUBETEXTURE;
    (*ppCubeTexture)->QueryInterface( IID_IDirect3DBaseTexture9, (LPVOID*)&NewEntry.pTexture );

    m_TextureCache.Add( NewEntry );
    return S_OK;
}
HRESULT CDXUTResourceCache::CreateVolumeTextureFromFile( LPDIRECT3DDEVICE9 pDevice, LPCTSTR pSrcFile, LPDIRECT3DVOLUMETEXTURE9 *ppVolumeTexture )
{
    return CreateVolumeTextureFromFileEx( pDevice, pSrcFile, D3DX_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT,
                                          0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED, D3DX_DEFAULT, D3DX_DEFAULT,
                                          0, NULL, NULL, ppVolumeTexture );
}
HRESULT CDXUTResourceCache::CreateVolumeTextureFromFileEx( LPDIRECT3DDEVICE9 pDevice, LPCTSTR pSrcFile, UINT Width, UINT Height, UINT Depth, UINT MipLevels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, DWORD Filter, DWORD MipFilter, D3DCOLOR ColorKey, D3DXIMAGE_INFO *pSrcInfo, PALETTEENTRY *pPalette, LPDIRECT3DVOLUMETEXTURE9 *ppTexture )
{
    // Search the cache for a matching entry.
    for( int i = 0; i < m_TextureCache.GetSize(); ++i )
    {
        DXUTCache_Texture &Entry = m_TextureCache[i];
        if( Entry.Location == DXUTCACHE_LOCATION_FILE &&
            !lstrcmpA( Entry.wszSource, pSrcFile ) &&
            Entry.Width == Width &&
            Entry.Height == Height &&
            Entry.Depth == Depth &&
            Entry.MipLevels == MipLevels &&
            Entry.Usage == Usage &&
            Entry.Format == Format &&
            Entry.Pool == Pool &&
            Entry.Type == D3DRTYPE_VOLUMETEXTURE )
        {
            // A match is found. Obtain the IDirect3DVolumeTexture9 interface and return that.
            return Entry.pTexture->QueryInterface( IID_IDirect3DVolumeTexture9, (LPVOID*)ppTexture );
        }
    }

    HRESULT hr;

    // No matching entry.  Load the resource and create a new entry.
    hr = D3DXCreateVolumeTextureFromFileEx( pDevice, pSrcFile, Width, Height, Depth, MipLevels, Usage, Format,
                                            Pool, Filter, MipFilter, ColorKey, pSrcInfo, pPalette, ppTexture );
    if( FAILED( hr ) )
        return hr;

    DXUTCache_Texture NewEntry;
    NewEntry.Location = DXUTCACHE_LOCATION_FILE;
    StringCchCopy( NewEntry.wszSource, MAX_PATH, pSrcFile );
    NewEntry.Width = Width;
    NewEntry.Height = Height;
    NewEntry.Depth = Depth;
    NewEntry.MipLevels = MipLevels;
    NewEntry.Usage = Usage;
    NewEntry.Format = Format;
    NewEntry.Pool = Pool;
    NewEntry.Type = D3DRTYPE_VOLUMETEXTURE;
    (*ppTexture)->QueryInterface( IID_IDirect3DBaseTexture9, (LPVOID*)&NewEntry.pTexture );

    m_TextureCache.Add( NewEntry );
    return S_OK;
}
HRESULT CDXUTResourceCache::CreateVolumeTextureFromResource( LPDIRECT3DDEVICE9 pDevice, HMODULE hSrcModule, LPCTSTR pSrcResource, LPDIRECT3DVOLUMETEXTURE9 *ppVolumeTexture )
{
    return CreateVolumeTextureFromResourceEx( pDevice, hSrcModule, pSrcResource, D3DX_DEFAULT, D3DX_DEFAULT,
                                              D3DX_DEFAULT, D3DX_DEFAULT, 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED,
                                              D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL, ppVolumeTexture );
}
HRESULT CDXUTResourceCache::CreateVolumeTextureFromResourceEx( LPDIRECT3DDEVICE9 pDevice, HMODULE hSrcModule, LPCTSTR pSrcResource, UINT Width, UINT Height, UINT Depth, UINT MipLevels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, DWORD Filter, DWORD MipFilter, D3DCOLOR ColorKey, D3DXIMAGE_INFO *pSrcInfo, PALETTEENTRY *pPalette, LPDIRECT3DVOLUMETEXTURE9 *ppVolumeTexture )
{
    // Search the cache for a matching entry.
    for( int i = 0; i < m_TextureCache.GetSize(); ++i )
    {
        DXUTCache_Texture &Entry = m_TextureCache[i];
        if( Entry.Location == DXUTCACHE_LOCATION_RESOURCE &&
            Entry.hSrcModule == hSrcModule &&
            !lstrcmpA( Entry.wszSource, pSrcResource ) &&
            Entry.Width == Width &&
            Entry.Height == Height &&
            Entry.Depth == Depth &&
            Entry.MipLevels == MipLevels &&
            Entry.Usage == Usage &&
            Entry.Format == Format &&
            Entry.Pool == Pool &&
            Entry.Type == D3DRTYPE_VOLUMETEXTURE )
        {
            // A match is found. Obtain the IDirect3DVolumeTexture9 interface and return that.
            return Entry.pTexture->QueryInterface( IID_IDirect3DVolumeTexture9, (LPVOID*)ppVolumeTexture );
        }
    }

    HRESULT hr;

    // No matching entry.  Load the resource and create a new entry.
    hr = D3DXCreateVolumeTextureFromResourceEx( pDevice, hSrcModule, pSrcResource, Width, Height, Depth, MipLevels, Usage,
                                                Format, Pool, Filter, MipFilter, ColorKey, pSrcInfo, pPalette, ppVolumeTexture );
    if( FAILED( hr ) )
        return hr;

    DXUTCache_Texture NewEntry;
    NewEntry.Location = DXUTCACHE_LOCATION_RESOURCE;
    NewEntry.hSrcModule = hSrcModule;
    StringCchCopy( NewEntry.wszSource, MAX_PATH, pSrcResource );
    NewEntry.Width = Width;
    NewEntry.Height = Height;
    NewEntry.Depth = Depth;
    NewEntry.MipLevels = MipLevels;
    NewEntry.Usage = Usage;
    NewEntry.Format = Format;
    NewEntry.Pool = Pool;
    NewEntry.Type = D3DRTYPE_VOLUMETEXTURE;
    (*ppVolumeTexture)->QueryInterface( IID_IDirect3DBaseTexture9, (LPVOID*)&NewEntry.pTexture );

    m_TextureCache.Add( NewEntry );
    return S_OK;
}
HRESULT CDXUTResourceCache::CreateFont( LPDIRECT3DDEVICE9 pDevice, UINT Height, UINT Width, UINT Weight, UINT MipLevels, BOOL Italic, DWORD CharSet, DWORD OutputPrecision, DWORD Quality, DWORD PitchAndFamily, LPCTSTR pFacename, LPD3DXFONT *ppFont )
{
    D3DXFONT_DESC Desc;
    
    Desc.Height = Height;
    Desc.Width = Width;
    Desc.Weight = Weight;
    Desc.MipLevels = MipLevels;
    Desc.Italic = Italic;
    Desc.CharSet = (BYTE)CharSet;
    Desc.OutputPrecision = (BYTE)OutputPrecision;
    Desc.Quality = (BYTE)Quality;
    Desc.PitchAndFamily = (BYTE)PitchAndFamily;
    StringCchCopy( Desc.FaceName, LF_FACESIZE, pFacename );

    return CreateFontIndirect( pDevice, &Desc, ppFont );
}
HRESULT CDXUTResourceCache::CreateFontIndirect( LPDIRECT3DDEVICE9 pDevice, CONST D3DXFONT_DESC *pDesc, LPD3DXFONT *ppFont )
{
    // Search the cache for a matching entry.
    for( int i = 0; i < m_FontCache.GetSize(); ++i )
    {
        DXUTCache_Font &Entry = m_FontCache[i];

        if( Entry.Width == pDesc->Width &&
            Entry.Height == pDesc->Height &&
            Entry.Weight == pDesc->Weight &&
            Entry.MipLevels == pDesc->MipLevels &&
            Entry.Italic == pDesc->Italic &&
            Entry.CharSet == pDesc->CharSet &&
            Entry.OutputPrecision == pDesc->OutputPrecision &&
            Entry.Quality == pDesc->Quality &&
            Entry.PitchAndFamily == pDesc->PitchAndFamily &&
            CompareString( LOCALE_USER_DEFAULT, NORM_IGNORECASE,
                           Entry.FaceName, -1,
                           pDesc->FaceName, -1 ) == CSTR_EQUAL )
        {
            // A match is found.  Increment the reference and return the ID3DXFont object.
            Entry.pFont->AddRef();
            *ppFont = Entry.pFont;
            return S_OK;
        }
    }

    HRESULT hr;

    // No matching entry.  Load the resource and create a new entry.
    hr = D3DXCreateFontIndirect( pDevice, pDesc, ppFont );
    if( FAILED( hr ) )
        return hr;

    DXUTCache_Font NewEntry;
    (D3DXFONT_DESC &)NewEntry = *pDesc;
    NewEntry.pFont = *ppFont;
    NewEntry.pFont->AddRef();

    m_FontCache.Add( NewEntry );
    return S_OK;
}
HRESULT CDXUTResourceCache::CreateEffectFromFile( LPDIRECT3DDEVICE9 pDevice, LPCTSTR pSrcFile, const D3DXMACRO *pDefines, LPD3DXINCLUDE pInclude, DWORD Flags, LPD3DXEFFECTPOOL pPool, LPD3DXEFFECT *ppEffect, LPD3DXBUFFER *ppCompilationErrors )
{
    // Search the cache for a matching entry.
    for( int i = 0; i < m_EffectCache.GetSize(); ++i )
    {
        DXUTCache_Effect &Entry = m_EffectCache[i];

        if( Entry.Location == DXUTCACHE_LOCATION_FILE &&
            !lstrcmpA( Entry.wszSource, pSrcFile ) &&
            Entry.dwFlags == Flags )
        {
            // A match is found.  Increment the ref coutn and return the ID3DXEffect object.
            *ppEffect = Entry.pEffect;
            (*ppEffect)->AddRef();
            return S_OK;
        }
    }

    HRESULT hr;

    // No matching entry.  Load the resource and create a new entry.
    hr = D3DXCreateEffectFromFile( pDevice, pSrcFile, pDefines, pInclude, Flags, pPool, ppEffect, ppCompilationErrors );
    if( FAILED( hr ) )
        return hr;

    DXUTCache_Effect NewEntry;
    NewEntry.Location = DXUTCACHE_LOCATION_FILE;
    StringCchCopy( NewEntry.wszSource, MAX_PATH, pSrcFile );
    NewEntry.dwFlags = Flags;
    NewEntry.pEffect = *ppEffect;
    NewEntry.pEffect->AddRef();

    m_EffectCache.Add( NewEntry );
    return S_OK;
}
HRESULT CDXUTResourceCache::CreateEffectFromResource( LPDIRECT3DDEVICE9 pDevice, HMODULE hSrcModule, LPCTSTR pSrcResource, const D3DXMACRO *pDefines, LPD3DXINCLUDE pInclude, DWORD Flags, LPD3DXEFFECTPOOL pPool, LPD3DXEFFECT *ppEffect, LPD3DXBUFFER *ppCompilationErrors )
{
    // Search the cache for a matching entry.
    for( int i = 0; i < m_EffectCache.GetSize(); ++i )
    {
        DXUTCache_Effect &Entry = m_EffectCache[i];

        if( Entry.Location == DXUTCACHE_LOCATION_RESOURCE &&
            Entry.hSrcModule == hSrcModule &&
            !lstrcmpA( Entry.wszSource, pSrcResource ) &&
            Entry.dwFlags == Flags )
        {
            // A match is found.  Increment the ref coutn and return the ID3DXEffect object.
            *ppEffect = Entry.pEffect;
            (*ppEffect)->AddRef();
            return S_OK;
        }
    }

    HRESULT hr;

    // No matching entry.  Load the resource and create a new entry.
    hr = D3DXCreateEffectFromResource( pDevice, hSrcModule, pSrcResource, pDefines, pInclude, Flags,
                                       pPool, ppEffect, ppCompilationErrors );
    if( FAILED( hr ) )
        return hr;

    DXUTCache_Effect NewEntry;
    NewEntry.Location = DXUTCACHE_LOCATION_RESOURCE;
    NewEntry.hSrcModule = hSrcModule;
    StringCchCopy( NewEntry.wszSource, MAX_PATH, pSrcResource );
    NewEntry.dwFlags = Flags;
    NewEntry.pEffect = *ppEffect;
    NewEntry.pEffect->AddRef();

    m_EffectCache.Add( NewEntry );
    return S_OK;
}
