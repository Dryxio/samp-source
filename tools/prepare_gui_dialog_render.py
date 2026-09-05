"""Reconstruct dialog refresh/focus and the verified R5 render adaptation."""
from prepare_checkpoint32 import definition
from verify import ROOT
def render_body(source):
    return definition(source,'CDXUTDialog::OnRender').replace('DrawText( wszOutput, &m_CapElement, &rc, true );','DrawText( wszOutput, &m_CapElement, &rc, false );')

def prepare():
    source=(ROOT/'vendor/upstream/saco/d3d9/common/DXUTgui.cpp').read_text()
    header='// Complete original dialog refresh and focus providers.\n#include "d3d9/common/dxstdafx.h"\n#include <new>\n'
    body='\n'.join(definition(source,n) for n in ('CDXUTDialog::Refresh','CDXUTDialog::FocusDefaultControl','CDXUTControl::Refresh'))
    (ROOT/'client/saco/closure_gui_dialog_render.cpp').write_text(header+body+'\n')
    for name,decl in [('refresh_time','double CDXUTDialog::s_fTimeRefresh = 0.0f;'),('pressed','CDXUTControl* CDXUTDialog::s_pControlPressed = NULL;')]:
        (ROOT/('client/saco/closure_store_gui_'+name+'.cpp')).write_text('#include "d3d9/common/dxstdafx.h"\n'+decl+'\n')
if __name__=='__main__':prepare()
