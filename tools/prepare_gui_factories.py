"""Complete original dialog control factories and shared initialization."""
from prepare_checkpoint32 import definition
from verify import ROOT
NAMES=['CDXUTDialog::InitControl','CDXUTDialog::AddControl','CDXUTSlider::SetRange','CDXUTDialog::AddSlider','CDXUTDialog::AddStatic','CDXUTDialog::AddButton','CDXUTDialog::AddCheckBox','CDXUTDialog::AddRadioButton']
def prepare():
    s=(ROOT/'vendor/upstream/saco/d3d9/common/DXUTgui.cpp').read_text()
    (ROOT/'client/saco/closure_gui_factories.cpp').write_text('#include "d3d9/common/dxstdafx.h"\n#include <new>\n'+'\n'.join(definition(s,n) for n in NAMES[:3]+['CDXUTStatic::SetText'])+'\n')
if __name__=='__main__':prepare()
