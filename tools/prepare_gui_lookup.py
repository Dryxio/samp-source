"""Complete original dialog lookup overloads."""
from prepare_checkpoint32 import definition
from verify import ROOT
def prepare():
 s=(ROOT/'vendor/upstream/saco/d3d9/common/DXUTgui.cpp').read_text()
 (ROOT/'client/saco/closure_gui_lookup.cpp').write_text('#include "d3d9/common/dxstdafx.h"\n'+'\n'.join(definition(s,'CDXUTDialog::GetControl',i) for i in range(2))+'\n')
if __name__=='__main__':prepare()
