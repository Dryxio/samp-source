"""Extract complete edit-control dependencies from the pinned DXUT source."""
from pathlib import Path
from prepare_checkpoint32 import definition
from verify import ROOT

SELECTED=[('ConvertWideToAnsi',None),('ConvertAnsiToWide',None),('CDXUTEditBox::GetTextA',None),('CDXUTEditBox::SetText',None),('CDXUTEditBox::PlaceCaret',None),('CUniBuffer::SetBufferSize',None),('CUniBuffer::SetText',0),('CUniBuffer::SetText',1),('CUniBuffer::CPtoX',None),('CUniBuffer::XtoCP',None),('CUniBuffer::Analyse',None)]
def prepare():
 source=ROOT/'vendor/upstream/saco/d3d9/common/DXUTgui.cpp'
 text=source.read_bytes().decode('latin1')
 header='// Complete definitions extracted from pinned DXUTgui.cpp by tools/prepare_gui_edit.py.\n#include "main.h"\n#include <strsafe.h>\n#define DXUT_MAX_EDITBOXLENGTH 0xFFFF\n'
 header+=definition(text,'RectWidth',0)+'\n'
 result=header+'\n\n'.join(definition(text,name,overload) for name,overload in SELECTED)+'\n'
 # R5's password mode is present in the verified layout but absent from the
 # older DXUT implementation. Keep the source snapshot immutable.
 helper="""WCHAR *MaskPasswordString(const WCHAR *text)
{
    static WCHAR masked[256];
    memset(masked, 0, 511);
    int length = (int)wcslen(text);
    int i = 0;
    if(length <= 255) {
        for(; i != length; i++) masked[i] = 0x25CF;
        masked[i] = 0;
    }
    return masked;
}
"""
 old=definition(text,'CUniBuffer::Analyse')
 start=old.index('    HRESULT hr = ');end=old.index('    if( SUCCEEDED( hr ) )',start)
 call=old[start:end].replace('    HRESULT hr = ','        hr = ').replace('DEFAULT_CHARSET,','-1,')
 password=call.replace('lstrlenW( m_pwszBuffer )','lstrlenW( MaskPasswordString(m_pwszBuffer) )').replace('                                       m_pwszBuffer,','                                       MaskPasswordString(m_pwszBuffer),')
 adapted=old[:start]+'    HRESULT hr;\n    if(!field_12) {\n'+call+'    } else {\n'+password+'    }\n'+old[end:]
 result=result.replace(old,helper+'\n'+adapted)
 result=result.replace('WCHAR szBuffer[2048];','WCHAR szBuffer[2049];')
 (ROOT/'client/saco/closure_gui_edit.cpp').write_bytes(result.encode('latin1'))
if __name__=='__main__':prepare()
