"""Original static/button/check/radio/slider constructors with verified R5 headers."""
from prepare_checkpoint32 import definition
from verify import ROOT
from prepare_gui_listbox import adapted, layout_header
from prepare_gui_control_lifetime import NAMES as BASE_NAMES
NAMES=['CDXUTStatic::CDXUTStatic','CDXUTButton::CDXUTButton','CDXUTCheckBox::CDXUTCheckBox','CDXUTRadioButton::CDXUTRadioButton','CDXUTSlider::CDXUTSlider','CDXUTSlider::ContainsPoint','CDXUTCheckBox::ContainsPoint']
def prepare():
    s=(ROOT/'vendor/upstream/saco/d3d9/common/DXUTgui.cpp').read_text()
    (ROOT/'client/saco/closure_gui_widget_lifetime.cpp').write_text('#include "d3d9/common/dxstdafx.h"\n#include <new>\n'+'#define SCROLLBAR_MINTHUMBSIZE 8\ninline int RectWidth( RECT &rc ) { return ( (rc).right - (rc).left ); }\ninline int RectHeight( RECT &rc ) { return ( (rc).bottom - (rc).top ); }\n'+'\n'.join(definition(s,n) for n in ['CDXUTScrollBar::Cap','CDXUTScrollBar::UpdateThumbRect','CDXUTScrollBar::SetTrackRange'])+'\n'+layout_header()+'\n'.join(definition(s,n) for n in ['CDXUTComboBox::Render','CDXUTComboBox::UpdateRects','CDXUTScrollBar::Scroll'])+'\n'+adapted(s,'CDXUTListBox::UpdateRects')+'\n'+'\n'.join(definition(s,n) for n in BASE_NAMES+NAMES+['CDXUTDialog::AddSlider','CDXUTDialog::AddStatic','CDXUTDialog::AddButton','CDXUTDialog::AddCheckBox','CDXUTDialog::AddRadioButton','CDXUTComboBox::CDXUTComboBox','CDXUTComboBox::SetTextColor','CDXUTComboBox::OnFocusOut','CDXUTComboBox::OnHotkey','CDXUTDialog::AddComboBox'])+'\n'+s[s.index('HRESULT CDXUTComboBox::AddItem'):s.index(definition(s,'CDXUTComboBox::SetSelectedByData'))+len(definition(s,'CDXUTComboBox::SetSelectedByData'))]+'\n'+definition(s,'CDXUTComboBox::~CDXUTComboBox')+'\n')
if __name__=='__main__':prepare()
