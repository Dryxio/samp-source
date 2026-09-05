// Complete original Uniscribe initialization and buffer lifetime methods.
#include "main.h"
#include <strsafe.h>
#define UNISCRIBE_DLLNAME "\\usp10.dll"

#define GETPROCADDRESS( Module, APIName, Temp ) \
    Temp = GetProcAddress( Module, #APIName ); \
    if( Temp ) \
        *(FARPROC*)&_##APIName = Temp


void CUniBuffer::Initialize()
{
    if( s_hDll ) // Only need to do once
        return;

    TCHAR wszPath[MAX_PATH+1];
    if( !::GetSystemDirectory( wszPath, MAX_PATH+1 ) )
        return;

    // Verify whether it is safe to concatenate these strings
    int len1 = lstrlen(wszPath);
    int len2 = lstrlen(UNISCRIBE_DLLNAME);
    if (len1 + len2 > MAX_PATH)
    {
        return;
    }

    // We have verified that the concatenated string will fit into wszPath,
    // so it is safe to concatenate them.
    StringCchCat( wszPath, MAX_PATH, UNISCRIBE_DLLNAME );

    s_hDll = LoadLibrary( wszPath );
    if( s_hDll )
    {
        FARPROC Temp;
        GETPROCADDRESS( s_hDll, ScriptApplyDigitSubstitution, Temp );
        GETPROCADDRESS( s_hDll, ScriptStringAnalyse, Temp );
        GETPROCADDRESS( s_hDll, ScriptStringCPtoX, Temp );
        GETPROCADDRESS( s_hDll, ScriptStringXtoCP, Temp );
        GETPROCADDRESS( s_hDll, ScriptStringFree, Temp );
        GETPROCADDRESS( s_hDll, ScriptString_pLogAttr, Temp );
        GETPROCADDRESS( s_hDll, ScriptString_pcOutChars, Temp );
    }
}
CUniBuffer::CUniBuffer( int nInitialSize )
{
    CUniBuffer::Initialize();  // ensure static vars are properly init'ed first

    m_nBufferSize = 0;
    m_pwszBuffer = NULL;
    m_bAnalyseRequired = true;
    m_Analysis = NULL;
    m_pFontNode = NULL;
    field_8 = 0;
	field_12 = 0;

    if( nInitialSize > 0 )
        SetBufferSize( nInitialSize );
}
CUniBuffer::~CUniBuffer()
{
    delete[] m_pwszBuffer;
    if( m_Analysis )
        _ScriptStringFree( &m_Analysis );
}
