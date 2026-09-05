"""Extract the original build-conditional DXUT diagnostic function."""
from prepare_checkpoint32 import definition
from verify import ROOT

def prepare():
 s=(ROOT/'vendor/upstream/saco/d3d9/common/DXUTmisc.cpp').read_text()
 (ROOT/'client/saco/closure_dxut_debug.cpp').write_text('// Original conditional debug output implementation.\n#include "d3d9/common/dxstdafx.h"\n'+definition(s,'DXUTOutputDebugStringA')+'\n'+definition(s,'DXTraceWrapper')+'\n')
if __name__=='__main__':prepare()
