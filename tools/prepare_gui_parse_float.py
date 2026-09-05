"""Complete original numeric parser for the verified wide edit buffer."""
from prepare_checkpoint32 import definition
from verify import ROOT
def prepare():
    s=(ROOT/'vendor/upstream/saco/d3d9/common/DXUTgui.cpp').read_text()
    start=s.index('#define IN_FLOAT_CHARSET');end=s.index('void CDXUTEditBox::ParseFloatArray',start)
    (ROOT/'client/saco/closure_gui_parse_float.cpp').write_text('#include "d3d9/common/dxstdafx.h"\n'+s[start:end]+definition(s,'CDXUTEditBox::ParseFloatArray')+'\n')
if __name__=='__main__':prepare()
