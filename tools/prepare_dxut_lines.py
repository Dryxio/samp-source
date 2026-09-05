"""Extract the complete original SDK line-manager family without body changes."""
from verify import ROOT

def extract(s):
    return s[s.index('CDXUTLineManager::CDXUTLineManager()'):s.index('CDXUTTextHelper::CDXUTTextHelper(')]

def prepare():
    source = (ROOT / 'vendor/upstream/saco/d3d9/common/DXUTmisc.cpp').read_text()
    (ROOT / 'client/saco/closure_dxut_lines.cpp').write_text(
        '// Complete original SDK line-manager implementation.\n'
        '#include "d3d9/common/dxstdafx.h"\n' + extract(source))

if __name__ == '__main__':
    prepare()
