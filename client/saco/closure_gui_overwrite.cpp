#include "d3d9/common/dxstdafx.h"
bool CUniBuffer::OverwriteChar(int nIndex, CHAR tchr)
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
		this[nIndex] = WideCharStr[0];
		return true;
	}
}
