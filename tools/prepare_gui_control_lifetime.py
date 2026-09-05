"""Materialize original control and scrollbar lifetime definitions."""
from prepare_checkpoint32 import definition
from verify import ROOT
NAMES=['CDXUTControl::CDXUTControl','CDXUTControl::~CDXUTControl','CDXUTScrollBar::CDXUTScrollBar','CDXUTScrollBar::~CDXUTScrollBar','CDXUTControl::SetTextColor']
def prepare():
    s=(ROOT/'vendor/upstream/saco/d3d9/common/DXUTgui.cpp').read_text()
    (ROOT/'client/saco/closure_gui_control_lifetime.cpp').write_text('#include "d3d9/common/dxstdafx.h"\n#include <new>\n'+'\n'.join(definition(s,n) for n in NAMES)+'\n')
if __name__=='__main__':prepare()
