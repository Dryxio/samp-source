"""Extract complete pinned DXUT drawing methods with the original vertex object."""
from prepare_checkpoint32 import definition
from verify import ROOT

def prepare():
 s=(ROOT/'vendor/upstream/saco/d3d9/common/DXUTgui.cpp').read_bytes().decode('latin1').replace('\r\n','\n')
 start=s.index('struct DXUT_SCREEN_VERTEX');end=s.index('\n\n',s.index('DWORD DXUT_SCREEN_VERTEX::FVF',start))
 header='// Complete pinned DXUT drawing definitions; no original machine-code bytes.\n#include "d3d9/common/dxstdafx.h"\n#include <new>\n#include "d3dhook/ID3DXFontHook.h"\n#undef min\n#undef max\n'+s[start:end]+'\n'
 selected=[('RectWidth',0),('RectHeight',0),('CDXUTDialog::GetTexture',None),('CDXUTDialog::DrawRect',None),('CDXUTDialog::DrawSprite',None),('CDXUTDialog::DrawPolyLine',None),('CDXUTDialog::DrawText',0),('CDXUTDialog::DrawText',1),('CDXUTDialog::CalcTextRect',0),('CDXUTDialog::CalcTextRect',1),('CDXUTDialogResourceManager::CreateFont',None)]
 text=header+'\n'.join(definition(s,n,overload) for n,overload in selected)+'\n'
 (ROOT/'client/saco/closure_gui_drawing.cpp').write_bytes(text.encode('latin1'))
if __name__=='__main__':prepare()
