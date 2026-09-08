// Original DXUT GUI methods, extracted from eval_gui_full.cpp.
// Ordinary C++, current R5 headers; Ob1 compilation still required.
#include "d3d9/common/dxstdafx.h"

HRESULT CDXUTStatic::GetTextCopy(PCHAR strDest, UINT bufferCount)
{
    if (strDest == NULL || bufferCount == 0)
        return E_INVALIDARG;
    StringCchCopy(strDest, bufferCount, m_strText);
    return S_OK;
}
HRESULT CDXUTEditBox::GetTextCopy(PWCHAR strDest, UINT bufferCount)
{
    assert(strDest);
    StringCchCopyW(strDest, bufferCount, m_Buffer.GetBuffer());
    return S_OK;
}
void CDXUTEditBox::ClearText()
{
    m_Buffer.Clear();
    m_nFirstVisible = 0;
    PlaceCaret(0);
    m_nSelStart = 0;
}
void CUniBuffer::Clear()
{
    *m_pwszBuffer = L'\0';
    m_bAnalyseRequired = true;
}
