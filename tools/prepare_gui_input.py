"""Extract original GUI input handlers and their real direct providers."""
from prepare_checkpoint32 import definition
from verify import ROOT
NAMES = [
    'CDXUTComboBox::HandleKeyboard', 'CDXUTComboBox::HandleMouse',
    'CDXUTSlider::HandleKeyboard', 'CDXUTSlider::HandleMouse',
    'CDXUTScrollBar::HandleMouse',
    'CDXUTEditBox::HandleKeyboard', 'CDXUTEditBox::HandleMouse',
    'CDXUTDialog::SendEvent', 'CDXUTDialog::ClearFocus',
    'CDXUTSlider::SetValueInternal', 'CDXUTSlider::ValueFromPos',
    'CDXUTEditBox::ResetCaretBlink', 'CDXUTEditBox::CopyToClipboard',
    'CDXUTEditBox::PasteFromClipboard', 'CDXUTEditBox::DeleteSelectionText',
    'CUniBuffer::RemoveChar', 'CUniBuffer::GetPriorItemPos',
    'CUniBuffer::GetNextItemPos', 'CUniBuffer::InsertString',
]
def body(source, name):
    text=definition(source,name)
    if name=='CDXUTEditBox::CopyToClipboard':
        text=text.replace('TCHAR','WCHAR').replace('CF_OEMTEXT','CF_UNICODETEXT')
    elif name=='CDXUTEditBox::PasteFromClipboard':
        text=text.replace('CF_OEMTEXT','CF_UNICODETEXT')
    elif name in ('CUniBuffer::RemoveChar','CUniBuffer::InsertString'):
        text=text.replace('sizeof(TCHAR)','sizeof(WCHAR)')
    return text

def prepare():
    source=(ROOT/'vendor/upstream/saco/d3d9/common/DXUTgui.cpp').read_text()
    header='// Complete original GUI input methods and providers.\n#include "d3d9/common/dxstdafx.h"\n#include <new>\n#ifndef WHEEL_DELTA\n#define WHEEL_DELTA 120\n#endif\n#define DXUT_MAX_EDITBOXLENGTH 0xFFFF\n'
    helpers='\n'.join(line for line in source.splitlines() if line.startswith(('inline int RectWidth(', 'inline int RectHeight(')))
    (ROOT/'client/saco/closure_gui_input.cpp').write_text(header+helpers+'\n'+'\n'.join(body(source,n) for n in NAMES)+'\n')
if __name__=='__main__':prepare()
