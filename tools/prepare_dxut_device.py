"""Extract complete device-compatibility predicate and format utilities."""
from prepare_checkpoint32 import definition
from verify import ROOT
FORMATS=['DXUTColorChannelBits','DXUTAlphaChannelBits','DXUTDepthBits','DXUTStencilBits']
def prepare():
 base=ROOT/'vendor/upstream/saco/d3d9/common';out=ROOT/'client/saco';s=(base/'DXUT.cpp').read_text()
 header='// Complete original DXUT device predicate.\n#include "d3d9/common/dxstdafx.h"\n#include <new>\nUINT DXUTDepthBits(D3DFORMAT);\nUINT DXUTStencilBits(D3DFORMAT);\n'
 (out/'closure_dxut_device.cpp').write_text(header+definition(s,'DXUTDoesDeviceComboMatchPreserveOptions')+'\n')
 s=(base/'DXUTenum.cpp').read_text();(out/'closure_dxut_formats.cpp').write_text('// Complete original DXUT format utilities.\n#include "d3d9/common/dxstdafx.h"\n'+'\n'.join(definition(s,n) for n in FORMATS)+'\n'+definition((base/'DXUTmisc.cpp').read_text(),'DXUTD3DFormatToString')+'\n')
 (out/'eval_dxut_enum.cpp').write_text('// Complete pinned enumeration discovery; include path only adapted.\n'+s.replace('#include "dxstdafx.h"','#include "d3d9/common/dxstdafx.h"'))
if __name__=='__main__':prepare()
