"""Reconstruct R5 edit rendering including the actual password branch."""
from prepare_checkpoint32 import definition
from verify import ROOT
def prepare():
 s=(ROOT/'vendor/upstream/saco/d3d9/common/DXUTgui.cpp').read_text();b=definition(s,'CDXUTEditBox::Render')
 start=b.index('    m_Elements.GetAt( 0 )->FontColor.Current = m_TextColor;')
 end=b.index('    //\n    /* Blink the caret',start)
 regular=b[start:end]
 password=regular.replace('m_Buffer.GetBuffer() + m_nFirstVisible','MaskPasswordString(m_Buffer.GetBuffer() + m_nFirstVisible)').replace('m_Buffer.GetBuffer() + nFirstToRender','MaskPasswordString(m_Buffer.GetBuffer() + nFirstToRender)')
 b=b[:start]+'    if(!field_126) {\n'+regular+'    } else {\n'+password+'    }\n\n'+b[end:]
 (ROOT/'client/saco/closure_gui_edit_render.cpp').write_text('#include "d3d9/common/dxstdafx.h"\n#include <new>\nWCHAR *MaskPasswordString(const WCHAR *text);\n'+b+'\n')
if __name__=='__main__':prepare()
