"""Recover R5 character dispatch, overwrite and caret advancement."""
from prepare_checkpoint32 import definition
from verify import ROOT
def prepare():
 s=(ROOT/'vendor/upstream/saco/d3d9/common/DXUTgui.cpp').read_text();b=definition(s,'CDXUTEditBox::MsgProc');start=b.index('\t\t\t\t\tbool bPlaceCaret;');end=b.index('                    ResetCaretBlink();',start)
 update='''                            PlaceCaret( m_nCaret + 1 );
                            m_nSelStart = m_nCaret;'''
 replacement="""                    bool bPlaceCaret = false;
                    if( !m_bInsertMode && m_nCaret < m_Buffer.GetTextSize() )
                    {
                        if( (WCHAR)wParam > 255 )
                        {
                            m_Buffer[m_nCaret] = (WCHAR)wParam;
"""+update+"""
                        }
                        else
                            bPlaceCaret = m_Buffer.OverwriteChar(m_nCaret, (CHAR)wParam);
                    }
                    else
                    {
                        bPlaceCaret = (WCHAR)wParam > 255 ?
                            m_Buffer.InsertChar(m_nCaret, (WCHAR)wParam) :
                            m_Buffer.InsertChar(m_nCaret, (CHAR)wParam);
                    }
                    if( bPlaceCaret )
                    {
"""+update+"""
                    }
"""
 b=(b[:start]+replacement+b[end:]).replace('(TCHAR)wParam','(WCHAR)wParam')
 (ROOT/'client/saco/closure_gui_edit_messages.cpp').write_text('#include "d3d9/common/dxstdafx.h"\n#define DXUT_MAX_EDITBOXLENGTH 0xFFFF\n'+b+'\n'+'\n'.join(definition(s,'CUniBuffer::InsertChar',i) for i in range(2))+'\n')
if __name__=='__main__':prepare()
