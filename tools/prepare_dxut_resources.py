"""Extract complete original cache creation methods and SDK GUID definitions."""
from prepare_checkpoint32 import definition
from verify import ROOT
NAMES=[f'Create{kind}From{location}{ex}' for kind in ['Texture','CubeTexture','VolumeTexture'] for location in ['File','Resource'] for ex in ['', 'Ex']]+['CreateFont','CreateFontIndirect','CreateEffectFromFile','CreateEffectFromResource']
def prepare():
 s=(ROOT/'vendor/upstream/saco/d3d9/common/DXUTmisc.cpp').read_text()
 (ROOT/'client/saco/closure_dxut_resources.cpp').write_text('// Complete pinned resource creation methods.\n#include "d3d9/common/dxstdafx.h"\n#include <strsafe.h>\n'+'\n'.join(definition(s,'CDXUTResourceCache::'+n) for n in NAMES)+'\n')
 (ROOT/'client/saco/closure_dxut_guids.cpp').write_text('// Complete interface identifiers from original SDK declarations.\n#include <windows.h>\n#include <initguid.h>\n#include <d3d9.h>\n')
if __name__=='__main__':prepare()
