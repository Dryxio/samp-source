"""Extract complete original IME lifetime, composition and indicator definitions."""
from prepare_checkpoint32 import definition
from verify import ROOT
NAMES=['Initialize','Uninitialize','RenderComposition','RenderIndicator','TruncateCompString']
STORES=['s_hDllImm32','s_hDllVer','s_ptCompString','s_nFirstTargetConv']
def prepare():
 s=(ROOT/'vendor/upstream/saco/d3d9/common/DXUTgui.cpp').read_text();out=ROOT/'client/saco'
 header='// Complete pinned IME lifetime and render definitions.\n#include "main.h"\nextern CGame *pGame;\n#include <new>\n#include <strsafe.h>\nextern DWORD dwImeWaitTick;\n'
 macros=s[s.index('#define GETPROCADDRESS'):s.index('// DXUT_MAX_EDITBOXLENGTH')]
 (out/'closure_gui_ime_render.cpp').write_text(header+macros+'\n'.join(definition(s,'CDXUTIMEEditBox::'+n) for n in NAMES)+'\n')
 for name in STORES:
  line=next(l for l in s.splitlines() if 'CDXUTIMEEditBox::'+name in l and not l.lstrip().startswith('//'))
  assert ';' in line
  (out/('closure_ime_store_'+name+'.cpp')).write_text('#include "d3d9/common/dxstdafx.h"\n'+line+'\n')
 line=next(l for l in s.splitlines() if l.startswith('DWORD dwImeWaitTick;'))
 (out/'closure_ime_store_wait_tick.cpp').write_text('#include "d3d9/common/dxstdafx.h"\n'+line+'\n')
if __name__=='__main__':prepare()
