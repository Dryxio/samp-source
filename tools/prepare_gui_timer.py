"""Extract complete source timer and remaining standalone IME helpers."""
from prepare_checkpoint32 import definition
from verify import ROOT
TIMER=['DXUTGetGlobalTimer',*[f'CDXUTTimer::{n}' for n in ['CDXUTTimer','Reset','Start','Stop','Advance','GetAbsoluteTime','GetTime','GetElapsedTime','IsStopped']]]
IME=['SendKey','StaticOnCreateDevice','SendCompString']
def prepare():
 base=ROOT/'vendor/upstream/saco/d3d9/common';out=ROOT/'client/saco';s=(base/'DXUTmisc.cpp').read_text()
 (out/'closure_gui_timer.cpp').write_text('// Complete pinned DXUT timer source.\n#include "d3d9/common/dxstdafx.h"\n#include <new>\n'+'\n'.join(definition(s,n) for n in TIMER)+'\n')
 s=(base/'DXUTgui.cpp').read_text()
 (out/'closure_gui_ime_helpers.cpp').write_text('// Complete pinned IME source helpers.\n#include "main.h"\nextern CGame *pGame;\n'+'\n'.join(definition(s,'CDXUTIMEEditBox::'+n) for n in IME)+'\n'+definition(s,'CUniBuffer::operator[]')+'\n')
 line=next(l for l in s.splitlines() if 'CDXUTIMEEditBox::s_hImcDef' in l and ';' in l)
 (out/'closure_ime_store_s_hImcDef.cpp').write_text('#include "d3d9/common/dxstdafx.h"\n'+line+'\n')
if __name__=='__main__':prepare()
