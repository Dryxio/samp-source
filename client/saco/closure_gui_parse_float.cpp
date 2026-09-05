#include "d3d9/common/dxstdafx.h"
#define IN_FLOAT_CHARSET( c ) \
    ( (c) == L'-' || (c) == L'.' || ( (c) >= L'0' && (c) <= L'9' ) )

void CDXUTEditBox::ParseFloatArray( float *pNumbers, int nCount )
{
    int nWritten = 0;  // Number of floats written
    const WCHAR *pToken, *pEnd;
    WCHAR wszToken[60];

    pToken = m_Buffer.GetBuffer();
    while( nWritten < nCount && *pToken != L'\0' )
    {
        // Skip leading spaces
        while( *pToken == L' ' )
            ++pToken;

        if( *pToken == L'\0' )
            break;

        // Locate the end of number
        pEnd = pToken;
        while( IN_FLOAT_CHARSET( *pEnd ) )
            ++pEnd;

        // Copy the token to our buffer
        int nTokenLen = __min( sizeof(wszToken) / sizeof(wszToken[0]) - 1, int(pEnd - pToken) );
        StringCchCopyW( wszToken, nTokenLen, pToken );
        *pNumbers = (float)wcstod( wszToken, NULL );
        ++nWritten;
        ++pNumbers;
        pToken = pEnd;
    }
}
