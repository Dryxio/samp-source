"""Extract complete original DXUT cursor bitmap/surface conversion."""
from prepare_checkpoint32 import definition
from verify import ROOT
def prepare():
 s=(ROOT/'vendor/upstream/saco/d3d9/common/DXUT.cpp').read_text()
 (ROOT/'client/saco/closure_dxut_cursor.cpp').write_text('// Complete original cursor conversion.\n#include "d3d9/common/dxstdafx.h"\n'+definition(s,'DXUTSetDeviceCursor')+'\n')
if __name__=='__main__':prepare()
