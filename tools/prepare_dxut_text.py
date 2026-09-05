"""Extract the complete original SDK TextHelper family."""
from verify import ROOT

def extract(source):
    return source[source.index('CDXUTTextHelper::CDXUTTextHelper('):source.index('IDirect3DDevice9* CDXUTDirectionWidget::s_pd3dDevice')]

def prepare():
    source=(ROOT/'vendor/upstream/saco/d3d9/common/DXUTmisc.cpp').read_text()
    (ROOT/'client/saco/closure_dxut_text.cpp').write_text('// Complete original SDK text helper.\n#include "d3d9/common/dxstdafx.h"\n'+extract(source))
if __name__=='__main__':prepare()
