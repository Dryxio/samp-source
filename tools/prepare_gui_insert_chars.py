"""Complete original wide and DBCS character insertion providers."""
from prepare_checkpoint32 import definition
from verify import ROOT
def prepare():
 s=(ROOT/'vendor/upstream/saco/d3d9/common/DXUTgui.cpp').read_text()
 (ROOT/'client/saco/closure_gui_insert_chars.cpp').write_text('#include "d3d9/common/dxstdafx.h"\n#define DXUT_MAX_EDITBOXLENGTH 0xFFFF\n'+'\n'.join(definition(s,'CUniBuffer::InsertChar',i) for i in range(2))+'\n')
if __name__=='__main__':prepare()
