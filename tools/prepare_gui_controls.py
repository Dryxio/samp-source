"""Extract complete pinned GUI control rendering and rectangle updates."""
from prepare_checkpoint32 import definition
from verify import ROOT
NAMES = [*(f'CDXUT{x}::Render' for x in ['ComboBox','Button','CheckBox','Slider','Static']), *(f'CDXUT{x}::UpdateRects' for x in ['Control','ComboBox','EditBox','Slider','CheckBox','ScrollBar']), 'CDXUTScrollBar::ShowItem', 'CDXUTScrollBar::Cap', 'CDXUTScrollBar::UpdateThumbRect', 'CDXUTScrollBar::Render', 'CDXUTScrollBar::Scroll']
def prepare():
 source=(ROOT/'vendor/upstream/saco/d3d9/common/DXUTgui.cpp').read_text()
 header='// Complete original DXUT control render/update definitions.\n#include "d3d9/common/dxstdafx.h"\n#include <new>\n#define SCROLLBAR_MINTHUMBSIZE 8\n#define SCROLLBAR_ARROWCLICK_DELAY 0.33\n#define SCROLLBAR_ARROWCLICK_REPEAT 0.05\n'
 (ROOT/'client/saco/closure_gui_controls.cpp').write_text(header+'\n'.join(line for line in source.splitlines() if line.startswith(('inline int RectWidth(', 'inline int RectHeight(')))+'\n'+'\n'.join(definition(source,n) for n in NAMES)+'\n')
if __name__=='__main__':prepare()
