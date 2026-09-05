"""Original static/button/check/radio/slider constructors with verified R5 headers."""
from prepare_checkpoint32 import definition
from verify import ROOT
from prepare_gui_control_lifetime import NAMES as BASE_NAMES
NAMES=['CDXUTStatic::CDXUTStatic','CDXUTButton::CDXUTButton','CDXUTCheckBox::CDXUTCheckBox','CDXUTRadioButton::CDXUTRadioButton','CDXUTSlider::CDXUTSlider','CDXUTSlider::ContainsPoint','CDXUTCheckBox::ContainsPoint']
def prepare():
    s=(ROOT/'vendor/upstream/saco/d3d9/common/DXUTgui.cpp').read_text()
    (ROOT/'client/saco/closure_gui_widget_lifetime.cpp').write_text('#include "d3d9/common/dxstdafx.h"\n#include <new>\n'+'\n'.join(definition(s,n) for n in BASE_NAMES+NAMES+['CDXUTDialog::AddSlider','CDXUTDialog::AddStatic','CDXUTDialog::AddButton','CDXUTDialog::AddCheckBox','CDXUTDialog::AddRadioButton'])+'\n')
if __name__=='__main__':prepare()
