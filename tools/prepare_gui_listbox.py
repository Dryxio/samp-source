"""Reconstruct ListBox keyboard selection using only R5-verified layout views."""
from prepare_checkpoint32 import definition
from verify import ROOT

def prepare():
    source=(ROOT/'vendor/upstream/saco/d3d9/common/DXUTgui.cpp').read_text()
    body=definition(source,'CDXUTListBox::HandleKeyboard')
    body=body.replace('m_ScrollBar','reinterpret_cast<R5ListBoxScrollView*>(this)->scrollbar')
    body=body.replace('pItem->bSelected','reinterpret_cast<R5ListBoxItemSelectionView*>(pItem)->selected')
    body=body.replace('m_Items[n]->bSelected','reinterpret_cast<R5ListBoxItemSelectionView*>(m_Items[n])->selected')
    body=body.replace('m_Items[m_nSelected]->bSelected','reinterpret_cast<R5ListBoxItemSelectionView*>(m_Items[m_nSelected])->selected')
    assert 'bSelected' not in body
    header='''// Partial views: reserved bytes are unknown, not reconstructed item fields.
// No allocation, sizeof-based access or coverage claim for these reserved ranges.
#include "d3d9/common/dxstdafx.h"
#include <new>
#include <stddef.h>
#pragma pack(push, 1)
struct R5ListBoxScrollView {
    unsigned char uncharacterized_prefix[0x5d];
    CDXUTScrollBar scrollbar;
};
struct R5ListBoxItemSelectionView {
    unsigned char uncharacterized_prefix[0x298];
    bool selected;
};
#pragma pack(pop)
typedef char VerifyScrollOffset[offsetof(R5ListBoxScrollView, scrollbar)==0x5d ? 1 : -1];
typedef char VerifySelectedOffset[offsetof(R5ListBoxItemSelectionView, selected)==0x298 ? 1 : -1];
'''
    (ROOT/'client/saco/closure_gui_listbox.cpp').write_text(header+body+'\n'+definition(source,'CDXUTScrollBar::HandleKeyboard')+'\n')
if __name__=='__main__':prepare()
