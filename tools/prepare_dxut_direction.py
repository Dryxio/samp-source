"""Extract the complete original SDK direction-widget family and static owners."""
from verify import ROOT

def extract(s):
    return s[s.index('IDirect3DDevice9* CDXUTDirectionWidget::s_pd3dDevice'):s.index('// Direct3D9 dynamic linking support')]

def prepare():
    source = (ROOT / 'vendor/upstream/saco/d3d9/common/DXUTmisc.cpp').read_text()
    (ROOT / 'client/saco/closure_dxut_direction.cpp').write_text(
        '// Complete original SDK direction-widget implementation.\n'
        '#include "d3d9/common/dxstdafx.h"\n' + extract(source))

if __name__ == '__main__':
    prepare()
