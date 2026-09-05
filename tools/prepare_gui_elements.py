"""Complete original element setter, numeric edit formatting and IME orientation."""
from prepare_checkpoint32 import definition
from verify import ROOT
NAMES=['CDXUTControl::SetElement','CDXUTEditBox::SetTextFloatArray','CDXUTIMEEditBox::GetReadingWindowOrientation']
def prepare():
    s=(ROOT/'vendor/upstream/saco/d3d9/common/DXUTgui.cpp').read_text()
    defines=s[s.index('#define LANG_CHT'):s.index('#define IMEID_CHT_VER42')]
    (ROOT/'client/saco/closure_gui_elements.cpp').write_text('#include "d3d9/common/dxstdafx.h"\n#include <new>\n'+defines+'\n'+'\n'.join(definition(s,n) for n in NAMES)+'\n')
    (ROOT/'client/saco/closure_ime_store_horizontal_reading.cpp').write_text('#include "d3d9/common/dxstdafx.h"\nbool CDXUTIMEEditBox::s_bHorizontalReading;\n')
if __name__=='__main__':prepare()
