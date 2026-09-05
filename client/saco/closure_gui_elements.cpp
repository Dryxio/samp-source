#include "d3d9/common/dxstdafx.h"
#include <new>
#define LANG_CHT            MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_TRADITIONAL)
#define LANG_CHS            MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_SIMPLIFIED)
#define _CHT_HKL            ( (HKL)(INT_PTR)0xE0080404 ) // New Phonetic
#define _CHT_HKL2           ( (HKL)(INT_PTR)0xE0090404 ) // New Chang Jie
#define _CHS_HKL            ( (HKL)(INT_PTR)0xE00E0804 ) // MSPY
#define MAKEIMEVERSION( major, minor )      ( (DWORD)( ( (BYTE)( major ) << 24 ) | ( (BYTE)( minor ) << 16 ) ) )


HRESULT CDXUTControl::SetElement( UINT iElement, CDXUTElement* pElement )
{
    HRESULT hr = S_OK;

    if( pElement == NULL )
        return E_INVALIDARG;

    // Make certain the array is this large
    for( UINT i=m_Elements.GetSize(); i <= iElement; i++ )
    {
        CDXUTElement* pNewElement = new CDXUTElement();
        if( pNewElement == NULL )
            return E_OUTOFMEMORY;

        hr = m_Elements.Add( pNewElement );
        if( FAILED(hr) )
        {
            SAFE_DELETE( pNewElement );
            return hr;
        }
    }

    // Update the data
    CDXUTElement* pCurElement = m_Elements.GetAt( iElement );
    *pCurElement = *pElement;
    
    return S_OK;
}
void CDXUTEditBox::SetTextFloatArray( const float *pNumbers, int nCount )
{
    TCHAR wszBuffer[512] = {0};
    TCHAR wszTmp[64];
    
    if( pNumbers == NULL )
        return;
        
    for( int i = 0; i < nCount; ++i )
    {
        StringCchPrintf( wszTmp, 64, "%.4f ", pNumbers[i] );
        StringCchCat( wszBuffer, 512, wszTmp );
    }

    // Don't want the last space
    if( nCount > 0 && strlen(wszBuffer) > 0 )
        wszBuffer[strlen(wszBuffer)-1] = 0;

    SetText( wszBuffer );
}
void CDXUTIMEEditBox::GetReadingWindowOrientation( DWORD dwId )
{
    s_bHorizontalReading = ( s_hklCurrent == _CHS_HKL ) || ( s_hklCurrent == _CHT_HKL2 ) || ( dwId == 0 );
    if( !s_bHorizontalReading && ( dwId & 0x0000FFFF ) == LANG_CHT )
    {
        TCHAR wszRegPath[MAX_PATH];
        HKEY hKey;
        DWORD dwVer = dwId & 0xFFFF0000;
        StringCchCopy( wszRegPath, MAX_PATH, "software\\microsoft\\windows\\currentversion\\" );
        StringCchCat( wszRegPath, MAX_PATH, ( dwVer >= MAKEIMEVERSION( 5, 1 ) ) ? "MSTCIPH" : "TINTLGNT" );
        LONG lRc = RegOpenKeyExA( HKEY_CURRENT_USER, wszRegPath, 0, KEY_READ, &hKey );
        if (lRc == ERROR_SUCCESS)
        {
            DWORD dwSize = sizeof(DWORD), dwMapping, dwType;
            lRc = RegQueryValueExA( hKey, "Keyboard Mapping", NULL, &dwType, (PBYTE)&dwMapping, &dwSize );
            if (lRc == ERROR_SUCCESS)
            {
                if ( ( dwVer <= MAKEIMEVERSION( 5, 0 ) && 
                       ( (BYTE)dwMapping == 0x22 || (BYTE)dwMapping == 0x23 ) )
                     ||
                     ( ( dwVer == MAKEIMEVERSION( 5, 1 ) || dwVer == MAKEIMEVERSION( 5, 2 ) ) &&
                       (BYTE)dwMapping >= 0x22 && (BYTE)dwMapping <= 0x24 )
                   )
                {
                    s_bHorizontalReading = true;
                }
            }
            RegCloseKey( hKey );
        }
    }
}
