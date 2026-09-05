"""Materialize pinned SDK source into the compiler source snapshot."""
from pathlib import Path
import hashlib
from verify import ROOT
SDK_SHA='ba86e00bcbdc631d9b3a44ed16b01678c1caf37dfda91f79cab34c7ae69b3f56'
def prepare():
 raw=(ROOT/'private/checkpoint31/multimon.h').read_bytes()
 if hashlib.sha256(raw).hexdigest()!=SDK_SHA:raise ValueError('Unexpected multimon SDK source')
 prefix='// Original MSVC2003 PlatformSDK multimon.h; SHA256 '+SDK_SHA+'\n#include "d3d9/common/dxstdafx.h"\n#define COMPILE_MULTIMON_STUBS\n'
 text=raw.decode('latin1').replace('\r\n','\n')
 # Original R5 calls kernel32 lstrcpyA with two arguments for this fixed DISPLAY literal.
 old='lstrcpyn(((MONITORINFOEX*)lpMonitorInfo)->szDevice, TEXT("DISPLAY"), (sizeof(((MONITORINFOEX*)lpMonitorInfo)->szDevice)/sizeof(TCHAR)));'
 assert text.count(old)==1
 text=text.replace(old,'lstrcpy(((MONITORINFOEX*)lpMonitorInfo)->szDevice, TEXT("DISPLAY"));')
 (ROOT/'client/saco/closure_dxut_multimon.cpp').write_text(prefix+text)
if __name__=='__main__':prepare()
