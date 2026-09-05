// Original conditional debug output implementation.
#include "d3d9/common/dxstdafx.h"
VOID DXUTOutputDebugStringA( LPCSTR strMsg, ... )
{
#if defined(DEBUG) | defined(_DEBUG)
    CHAR strBuffer[512];
    
    va_list args;
    va_start(args, strMsg);
    StringCchVPrintfA( strBuffer, 512, strMsg, args );
    strBuffer[511] = '\0';
    va_end(args);

    OutputDebugStringA( strBuffer );
#else
    UNREFERENCED_PARAMETER(strMsg);
#endif
}
HRESULT WINAPI DXTraceWrapper( const char* strFile, DWORD dwLine, HRESULT hr, const char* strMsg, BOOL bPopMsgBox )
{
#ifdef _DEBUG
	TCHAR szBuffer[1024];
	sprintf(szBuffer, "DXTrace: %s (%d) returned 0x%x, msg=%s", strFile, dwLine, hr, strMsg);
	OutputDebugString(szBuffer);
#endif
	return 0;
}
