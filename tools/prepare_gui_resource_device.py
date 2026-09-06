"""Original complete GUI resource manager device lifecycle candidates."""
from prepare_checkpoint32 import definition
from verify import ROOT

NAMES = tuple('CDXUTDialogResourceManager::' + name for name in (
    'CDXUTDialogResourceManager', 'OnCreateDevice', 'OnResetDevice',
    'OnLostDevice', 'OnDestroyDevice'))

def prepare():
    source = (ROOT / 'vendor/upstream/saco/d3d9/common/DXUTgui.cpp').read_text()
    (ROOT / 'client/saco/closure_gui_resource_device.cpp').write_text(
        '#include "d3d9/common/dxstdafx.h"\n' +
        '\n'.join(definition(source, name) for name in NAMES) + '\n')

if __name__ == '__main__':
    prepare()
