"""Reconstruct ListBox keyboard selection using only R5-verified layout views."""
from prepare_checkpoint32 import definition
from verify import ROOT

import re

def adapted(source,name):
    body=definition(source,name)
    body=body.replace('m_ScrollBar','reinterpret_cast<R5ListBoxScrollView*>(this)->scrollbar')
    body=body.replace('m_rcText','reinterpret_cast<R5ListBoxScrollView*>(this)->text')
    body=body.replace('m_rcSelection','reinterpret_cast<R5ListBoxScrollView*>(this)->selection')
    body=re.sub(r'(m_Items\.GetAt\([^)]*\)|m_Items\[[^\]]+\]|\b(?:pItem|pSelItem))\s*->bSelected',lambda m:'reinterpret_cast<R5ListBoxItemSelectionView*>('+m[1]+')->selected',body)
    if name=='CDXUTListBox::HandleMouse':
        assert body.count('m_bDrag = true;')==1
        body=body.replace('m_bDrag = true;','')
        assert body.count('if( m_nSelected != -1 )')==1
        body=body.replace('if( m_nSelected != -1 )','if( m_nSelected < m_Items.GetSize() && m_nSelected != -1 )')
    assert 'bSelected' not in body
    return body

def prepare():
    source=(ROOT/'vendor/upstream/saco/d3d9/common/DXUTgui.cpp').read_text()
    body='\n'.join(adapted(source,'CDXUTListBox::'+n) for n in ('HandleKeyboard','HandleMouse'))
    header='''// Partial views: reserved bytes are unknown, not reconstructed item fields.
// No allocation, sizeof-based access or coverage claim for these reserved ranges.
#include "d3d9/common/dxstdafx.h"
#include <new>
#include <stddef.h>
#ifndef WHEEL_DELTA
#define WHEEL_DELTA 120
#endif
#pragma pack(push, 1)
struct R5ListBoxScrollView {
    unsigned char uncharacterized_prefix[0x5d];
    CDXUTScrollBar scrollbar;
    RECT text;
    RECT selection;
};
struct R5ListBoxItemSelectionView {
    unsigned char uncharacterized_prefix[0x298];
    bool selected;
};
#pragma pack(pop)
typedef char VerifyScrollOffset[offsetof(R5ListBoxScrollView, scrollbar)==0x5d ? 1 : -1];
typedef char VerifyTextOffset[offsetof(R5ListBoxScrollView, text)==0x10f ? 1 : -1];
typedef char VerifySelectionOffset[offsetof(R5ListBoxScrollView, selection)==0x11f ? 1 : -1];
typedef char VerifySelectedOffset[offsetof(R5ListBoxItemSelectionView, selected)==0x298 ? 1 : -1];
'''
    (ROOT/'client/saco/closure_gui_listbox.cpp').write_text(header+body+'\n'+definition(source,'CDXUTScrollBar::HandleKeyboard')+'\n')
if __name__=='__main__':prepare()
