"""Extract complete original device-settings selection and resolution logic."""
from prepare_checkpoint32 import definition
from verify import ROOT
NAMES=['DXUTFindValidResolution','DXUTBuildOptimalDeviceSettings','DXUTBuildValidDeviceSettings']
def prepare():
 s=(ROOT/'vendor/upstream/saco/d3d9/common/DXUT.cpp').read_text()
 header='// Complete pinned device settings definitions.\n#include "d3d9/common/dxstdafx.h"\nUINT DXUTColorChannelBits(D3DFORMAT);\nUINT DXUTDepthBits(D3DFORMAT);\nUINT DXUTStencilBits(D3DFORMAT);\n'
 (ROOT/'client/saco/closure_dxut_settings.cpp').write_text(header+'\n'.join(definition(s,n) for n in NAMES)+'\n')
if __name__=='__main__':prepare()
