"""Extract complete original DXUT state, lifetime and public accessors."""
import re
from prepare_checkpoint32 import definition
from verify import ROOT
CORE=['DXUTShutdown','DXUTCleanup3DEnvironment','DXUTAllowShortcutKeys','DXUTDisplayErrorMessage','DXUTIsWindowed','DXUTGetCmdParam','DXUTSetConstantFrameTime','DXUTParseCommandLine','DXUTInit','DXUTPrepareEnumerationObject','DXUTUpdateBackBufferDesc','DXUTUpdateDeviceStats','DXUTUpdateStaticFrameStats','DXUTCreate3DEnvironment','DXUTReset3DEnvironment','DXUTPause','DXUTGetDeviceSettings','DXUTUpdateDeviceSettingsWithOverrides','DXUTInitHWCursor','DXUTGetDesktopResolution']
def body(source,name):
 # Anchor actual top-level definitions; call expressions can fool the legacy extractor.
 match=re.search(r'(?m)^(?:HRESULT|void|bool|DXUTDeviceSettings|CD3DEnumeration\*) '+re.escape(name)+r'\s*\([^;{}]*\)\s*\{',source)
 if not match:raise ValueError('Missing top-level definition '+name)
 return definition(source[match.start():],name,0)
def prepare():
 s=(ROOT/'vendor/upstream/saco/d3d9/common/DXUT.cpp').read_text()
 state=s[s.index('CRITICAL_SECTION g_cs;'):s.index('// Internal functions forward declarations')]
 access=s[s.index('IDirect3D9* DXUTGetD3DObject()'):s.index('bool DXUTIsKeyDown(')]
 names=re.findall(r'\b(DXUT\w+)\s*\([^;{}]*\)\s*\{',access)
 header='// Complete pinned framework state/lifetime and public accessors.\n#include "d3d9/common/dxstdafx.h"\n#include <strsafe.h>\n#undef GetSystemMetrics\nvoid DXUTCleanup3DEnvironment(bool = true);\nvoid DXUTDisplayErrorMessage(HRESULT);\nvoid DXUTAllowShortcutKeys(bool);\nCD3DEnumeration* DXUTPrepareEnumerationObject(bool = false);\nHRESULT DXUTSetDeviceCursor(IDirect3DDevice9*,HCURSOR,bool);\nvoid DXUTUpdateBackBufferDesc();\nvoid DXUTUpdateDeviceStats(D3DDEVTYPE,DWORD,D3DADAPTER_IDENTIFIER9*);\ntypedef DECLSPEC_IMPORT UINT (WINAPI* LPTIMEBEGINPERIOD)(UINT);\n'
 (ROOT/'client/saco/closure_dxut_state.cpp').write_text(header+state+'\n'+access+'\n'+'\n'.join(body(s,n) for n in CORE)+'\n')
 return names
if __name__=='__main__':print(prepare())
