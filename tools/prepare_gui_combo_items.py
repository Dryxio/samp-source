"""Complete ComboBox item ownership, lookup and selection methods."""
from prepare_checkpoint32 import definition
from verify import ROOT
NAMES=['AddItem','RemoveItem','RemoveAllItems','ContainsItem','FindItem','GetSelectedData','GetSelectedItem','GetItemData','SetSelectedByIndex','SetSelectedByText','SetSelectedByData']
def prepare():
    s=(ROOT/'vendor/upstream/saco/d3d9/common/DXUTgui.cpp').read_text()
    start=s.index('HRESULT CDXUTComboBox::AddItem');end=s.index('//--------------------------------------------------------------------------------------',s.index('HRESULT CDXUTComboBox::SetSelectedByData'))
    # Complete contiguous item block, preserving both GetItemData overloads.
    last=definition(s,'CDXUTComboBox::SetSelectedByData')
    end=s.index(last,start)+len(last)
    (ROOT/'client/saco/closure_gui_combo_items.cpp').write_text('#include "d3d9/common/dxstdafx.h"\n#include <new>\n'+s[start:end]+'\n')
if __name__=='__main__':prepare()
