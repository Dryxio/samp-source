"""Extract complete pinned dialog-default and media-search source families."""
from prepare_checkpoint32 import definition
from verify import ROOT

def prepare():
 base=ROOT/'vendor/upstream/saco/d3d9/common';s=(base/'DXUTgui.cpp').read_text();out=ROOT/'client/saco'
 header='// Complete original dialog default/resource definitions.\n#include "d3d9/common/dxstdafx.h"\n#include <new>\n#undef min\n#undef max\nint GetUIFontSize();\nint GetFontWeight();\nchar *GetFontFace();\n'
 names=['DXUTBlendColor::Init','DXUTBlendColor::Blend','CDXUTElement::SetTexture','CDXUTElement::SetFont','CDXUTElement::Refresh','CDXUTDialog::SetDefaultElement','CDXUTDialog::SetTexture','CDXUTDialog::SetFont','CDXUTDialog::UpdateFont','CDXUTDialog::InitDefaultElements','CDXUTDialogResourceManager::AddFont','CDXUTDialogResourceManager::AddTexture','CDXUTDialogResourceManager::CreateTexture']
 (out/'closure_gui_defaults.cpp').write_text(header+'\n'.join(definition(s,n) for n in names)+'\n')
 s=(base/'DXUTmisc.cpp').read_text();header='// Complete original DXUT media-search definitions.\n#include "d3d9/common/dxstdafx.h"\n#include <new>\n#include <strsafe.h>\nbool DXUTFindMediaSearchTypicalDirs(TCHAR*,int,LPCTSTR,TCHAR*,TCHAR*);\nbool DXUTFindMediaSearchParentDirs(TCHAR*,int,TCHAR*,TCHAR*);\n'
 names=['DXUTMediaSearchPath','DXUTGetMediaSearchPath','DXUTSetMediaSearchPath','DXUTFindDXSDKMediaFileCch','DXUTFindMediaSearchTypicalDirs','DXUTFindMediaSearchParentDirs']
 (out/'closure_gui_media.cpp').write_text(header+'\n'.join(definition(s,n) for n in names)+'\n')
if __name__=='__main__':prepare()
