"""Reconstruct dialog message routing and original focus/hover providers."""
from prepare_checkpoint32 import definition
from verify import ROOT
NAMES=['CDXUTDialog::GetControlAtPoint','CDXUTDialog::OnMouseMove','CDXUTDialog::OnCycleFocus','CDXUTDialog::GetNextControl','CDXUTDialog::GetPrevControl']
def message_body(source):
    body=definition(source,'CDXUTDialog::MsgProc')
    start=body.index('    // If caption is enable, check for clicks in the caption area.')
    end=body.index('    // If the dialog is minimized,',start)
    body=body[:start]+body[end:]
    body=body.replace('\t\t\tOutputDebugString("CDXUTDialog::MsgProc(MOUSE)");','')
    return body

def prepare():
    source=(ROOT/'vendor/upstream/saco/d3d9/common/DXUTgui.cpp').read_text()
    header='// R5 message routing with complete original focus/hover methods.\n#include "d3d9/common/dxstdafx.h"\n#include <new>\n'
    # Original local compatibility definitions used by the message switch.
    start=source.index('#ifndef WM_XBUTTONDOWN');end=source.index('// Minimum scroll bar thumb size',start)
    (ROOT/'client/saco/closure_gui_messages.cpp').write_text(header+source[start:end]+message_body(source)+'\n'+'\n'.join(definition(source,n) for n in NAMES)+'\n')
if __name__=='__main__':prepare()
