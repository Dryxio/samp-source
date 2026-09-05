"""Reconstruct original GUI button input with the verified R5 press event."""
from prepare_checkpoint32 import definition
from verify import ROOT
NAMES=[*(f'CDXUT{c}::{method}' for c in ('Button','CheckBox','RadioButton') for method in ('HandleKeyboard','HandleMouse')), 'CDXUTCheckBox::SetCheckedInternal','CDXUTRadioButton::SetCheckedInternal','CDXUTDialog::ClearRadioButtonGroup']
def body(source,name):
    text=definition(source,name)
    if name=='CDXUTButton::HandleMouse':
        text=text.replace('\tOutputDebugString("CDXUTButton::HandleMouse");','')
        needle='SetCapture( DXUTGetHWND() );'
        assert text.count(needle)==1
        text=text.replace(needle,needle+'\n                m_pDialog->SendEvent( EVENT_BUTTON_PRESSED_R5, true, this );')
    return text

def prepare():
    source=(ROOT/'vendor/upstream/saco/d3d9/common/DXUTgui.cpp').read_text()
    header='// Original button input with R5 verified press event.\n#include "d3d9/common/dxstdafx.h"\n#include <new>\n#define EVENT_BUTTON_PRESSED_R5 0x0102\n'
    (ROOT/'client/saco/closure_gui_buttons.cpp').write_text(header+'\n'.join(body(source,n) for n in NAMES)+'\n')
if __name__=='__main__':prepare()
