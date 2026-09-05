"""Extract complete original DXUT state, lifetime and public accessors."""
import re
from prepare_checkpoint32 import definition
from verify import ROOT
CORE=['DXUTShutdown','DXUTCleanup3DEnvironment','DXUTAllowShortcutKeys','DXUTDisplayErrorMessage','DXUTIsWindowed','DXUTGetCmdParam','DXUTSetConstantFrameTime','DXUTParseCommandLine','DXUTInit','DXUTPrepareEnumerationObject']
def prepare():
 s=(ROOT/'vendor/upstream/saco/d3d9/common/DXUT.cpp').read_text()
 state=s[s.index('CRITICAL_SECTION g_cs;'):s.index('// Internal functions forward declarations')]
 access=s[s.index('IDirect3D9* DXUTGetD3DObject()'):s.index('bool DXUTIsKeyDown(')]
 names=re.findall(r'\b(DXUT\w+)\s*\([^;{}]*\)\s*\{',access)
 header='// Complete pinned framework state/lifetime and public accessors.\n#include "d3d9/common/dxstdafx.h"\n#include <strsafe.h>\n#undef GetSystemMetrics\nvoid DXUTCleanup3DEnvironment(bool);\nvoid DXUTDisplayErrorMessage(HRESULT);\nvoid DXUTAllowShortcutKeys(bool);\ntypedef DECLSPEC_IMPORT UINT (WINAPI* LPTIMEBEGINPERIOD)(UINT);\n'
 (ROOT/'client/saco/closure_dxut_state.cpp').write_text(header+state+'\n'+access+'\n'+'\n'.join(definition(s[s.index('bool '+n+'('):],n,0) if n in ('DXUTIsWindowed','DXUTGetCmdParam') else definition(s,n) for n in CORE)+'\n')
 return names
if __name__=='__main__':print(prepare())
