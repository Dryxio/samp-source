"""Reconstruct complete dialog lifetime and removal methods from pinned SDK source."""
from prepare_checkpoint32 import definition
from verify import ROOT
NAMES=['CDXUTDialog::CDXUTDialog','CDXUTDialog::~CDXUTDialog','CDXUTDialog::RemoveControl','CDXUTDialog::RemoveAllControls']
def prepare():
    source=(ROOT/'vendor/upstream/saco/d3d9/common/DXUTgui.cpp').read_text()
    (ROOT/'client/saco/closure_gui_lifecycle.cpp').write_text('// Complete original dialog lifetime and removal providers.\n#include "d3d9/common/dxstdafx.h"\n#include <new>\n'+'\n'.join(definition(source,n) for n in NAMES)+'\n')
if __name__=='__main__':prepare()
