#include "d3d9/common/dxstdafx.h"
#define DXUT_MAX_EDITBOXLENGTH 0xFFFF
bool CUniBuffer::InsertChar( int nIndex, WCHAR tchr )
{
    assert( nIndex >= 0 );

    if( nIndex < 0 || nIndex > lstrlenW( m_pwszBuffer ) )
        return false;  // invalid index

    // Check for maximum length allowed
    if( GetTextSize() + 1 >= DXUT_MAX_EDITBOXLENGTH )
        return false;

    if( lstrlenW( m_pwszBuffer ) + 1 >= m_nBufferSize )
    {
        if( !SetBufferSize( -1 ) )
            return false;  // out of memory
    }

    assert( m_nBufferSize >= 2 );

    // Shift the characters after the index, start by copying the null terminator
    WCHAR* dest = m_pwszBuffer + lstrlenW(m_pwszBuffer)+1;
    WCHAR* stop = m_pwszBuffer + nIndex;
    WCHAR* src = dest - 1;

    while( dest > stop )
    {
        *dest-- = *src--;
    }

    // Set new character
    m_pwszBuffer[ nIndex ] = tchr;
    m_bAnalyseRequired = true;

    return true;
}
bool CUniBuffer::InsertChar(int nIndex, CHAR tchr)
{
	WCHAR WideCharStr[2];
	ZeroMemory(&WideCharStr[0], sizeof(WideCharStr));

	if(field_8)
	{
		CHAR MultiByteStr[3];
		MultiByteStr[0] = field_8;
		MultiByteStr[1] = tchr;
		MultiByteStr[2] = 0;
		MultiByteToWideChar(CP_ACP, 0, MultiByteStr, 2, WideCharStr, 1);
		InsertChar(nIndex, WideCharStr[0]);
		field_8 = 0;
		return true;
	}
	else if(IsDBCSLeadByteEx(CP_ACP, tchr))
	{
		field_8 = tchr;
		return false;
	}
	else
	{
		MultiByteToWideChar(CP_ACP, 0, &tchr, 1, WideCharStr, 1);
		InsertChar(nIndex, WideCharStr[0]);
		return true;
	}
}
