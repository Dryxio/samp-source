"""Original SDK resource teardown candidates; acceptance is separate."""
from prepare_checkpoint32 import definition
from verify import ROOT

NAMES = ('CUniBuffer::Uninitialize', 'CDXUTDialogResourceManager::~CDXUTDialogResourceManager')

def prepare():
    source = (ROOT / 'vendor/upstream/saco/d3d9/common/DXUTgui.cpp').read_text()
    header = '#include "d3d9/common/dxstdafx.h"\n#define PLACEHOLDERPROC(APIName) _##APIName = Dummy_##APIName\n'
    (ROOT / 'client/saco/closure_gui_resource_teardown.cpp').write_text(
        header + '\n'.join(definition(source, name) for name in NAMES) + '\n')

if __name__ == '__main__':
    prepare()
