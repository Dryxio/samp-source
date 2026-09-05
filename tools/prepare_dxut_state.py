"""Extract complete original DXUT state, lifetime and public accessors."""
import re
from prepare_checkpoint32 import definition
from verify import ROOT
CORE=['DXUTShutdown','DXUTCleanup3DEnvironment','DXUTAllowShortcutKeys','DXUTDisplayErrorMessage','DXUTIsWindowed','DXUTGetCmdParam','DXUTSetConstantFrameTime','DXUTParseCommandLine','DXUTInit','DXUTPrepareEnumerationObject','DXUTUpdateBackBufferDesc','DXUTUpdateDeviceStats','DXUTUpdateStaticFrameStats','DXUTCreate3DEnvironment','DXUTReset3DEnvironment','DXUTPause','DXUTGetDeviceSettings','DXUTUpdateDeviceSettingsWithOverrides','DXUTInitHWCursor','DXUTGetDesktopResolution','DXUTChangeDevice','DXUTCreateWindow','DXUTSetWindow','DXUTCreateDevice','DXUTCreateDeviceFromSettings','DXUTSetDevice','DXUTFindAdapterFormat','DXUTStaticWndProc','DXUTCheckForWindowSizeChange','DXUTCheckForWindowChangingMonitors','DXUTGetAdapterOrdinalFromMonitor','DXUTToggleFullScreen','DXUTToggleREF']
def body(source,name):
 # Anchor actual top-level definitions; call expressions can fool the legacy extractor.
 match=re.search(r'(?m)^(?:HRESULT|void|bool|LRESULT CALLBACK|DXUTDeviceSettings|CD3DEnumeration\*) '+re.escape(name)+r'\s*\([^;{}]*\)\s*\{',source)
 if not match:raise ValueError('Missing top-level definition '+name)
 result=definition(source[match.start():],name,0)
 if name=='DXUTChangeDevice':
  # R5 calls Windowed first, then compares FullScreen against that saved handle.
  index=[0]
  def sequence(m):
   index[0]+=1;var='hWindowedForCompare'+str(index[0])
   return 'HWND '+var+' = DXUTGetHWNDDeviceWindowed();\n        if( DXUTGetHWNDDeviceFullScreen() '+m.group(1)+' '+var+' )'
  result=re.sub(r'if\( DXUTGetHWNDDeviceFullScreen\(\) (==|!=) DXUTGetHWNDDeviceWindowed\(\) \)',sequence,result)
  assert index[0]==3
  # Original IAT identity is SetWindowLongW for style updates.
  result=result.replace('SetWindowLong(', 'SetWindowLongW(')
 return result
def prepare():
 s=(ROOT/'vendor/upstream/saco/d3d9/common/DXUT.cpp').read_text()
 state=s[s.index('CRITICAL_SECTION g_cs;'):s.index('// Internal functions forward declarations')]
 access=s[s.index('IDirect3D9* DXUTGetD3DObject()'):s.index('bool DXUTIsKeyDown(')]
 names=re.findall(r'\b(DXUT\w+)\s*\([^;{}]*\)\s*\{',access)
 header='// Complete pinned framework state/lifetime and public accessors.\n#include "d3d9/common/dxstdafx.h"\n#include <strsafe.h>\n#undef GetSystemMetrics\n#define DXUT_MIN_WINDOW_SIZE_X 200\n#define DXUT_MIN_WINDOW_SIZE_Y 200\n'
 declarations=s[s.index('typedef IDirect3D9* (WINAPI* LPDIRECT3DCREATE9)'):s.index('// External callback setup functions')]
 header+=declarations+'\n'
 (ROOT/'client/saco/closure_dxut_state.cpp').write_text(header+state+'\n'+access+'\n'+'\n'.join(body(s,n) for n in CORE)+'\n')
 return names
if __name__=='__main__':print(prepare())
