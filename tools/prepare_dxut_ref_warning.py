"""Extract the complete original REF warning and its actual dialog callback."""
from prepare_checkpoint32 import definition
from verify import ROOT

def prepare():
    source=(ROOT/'vendor/upstream/saco/d3d9/common/DXUTmisc.cpp').read_text()
    header='// Complete original SDK REF warning and callback.\n#include "d3d9/common/dxstdafx.h"\nINT_PTR CALLBACK DisplaySwitchToREFWarningProc(HWND,UINT,WPARAM,LPARAM);\n'
    (ROOT/'client/saco/closure_dxut_ref_warning.cpp').write_text(header+definition(source,'DXUTDisplaySwitchingToREFWarning')+'\n'+definition(source,'DisplaySwitchToREFWarningProc')+'\n')
if __name__=='__main__':prepare()
