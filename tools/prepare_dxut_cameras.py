"""Extract complete adjacent original arcball/camera/cubemap implementations."""
from verify import ROOT

def extract(s):
 return s[s.index('CD3DArcBall::CD3DArcBall()'):s.index('// Returns the string for the given D3DFORMAT.')]
def prepare():
 s=(ROOT/'vendor/upstream/saco/d3d9/common/DXUTmisc.cpp').read_text()
 (ROOT/'client/saco/closure_dxut_cameras.cpp').write_text('// Complete original camera/arcball family.\n#include "d3d9/common/dxstdafx.h"\n'+extract(s))
if __name__=='__main__':prepare()
