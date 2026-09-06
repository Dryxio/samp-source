"""Reconstruct R5 byte overwrite helper, preserving its observed object assignment."""
from prepare_checkpoint32 import definition
from verify import ROOT
def prepare():
 p=ROOT/'vendor/upstream/saco/d3d9/common/DXUTgui.h'
 b=p.read_bytes();needle=b'    bool InsertChar( int nIndex, CHAR tchr );'
 if b'bool OverwriteChar(' not in b:
  assert needle in b;p.write_bytes(b.replace(needle,needle+b'\r\n    bool OverwriteChar( int nIndex, CHAR tchr ); // Recovered R5 helper; no layout change.'))
 s=(ROOT/'vendor/upstream/saco/d3d9/common/DXUTgui.cpp').read_text()
 b=definition(s,'CUniBuffer::InsertChar',1).replace('CUniBuffer::InsertChar(int nIndex, CHAR tchr)','CUniBuffer::OverwriteChar(int nIndex, CHAR tchr)')
 old='\t\tInsertChar(nIndex, WideCharStr[0]);';at=b.rindex(old);b=b[:at]+b[at:].replace(old,'\t\tthis[nIndex] = WideCharStr[0];',1)
 (ROOT/'client/saco/closure_gui_overwrite.cpp').write_text('#include "d3d9/common/dxstdafx.h"\n'+b+'\n')
if __name__=='__main__':prepare()
