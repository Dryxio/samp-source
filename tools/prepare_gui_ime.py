"""Extract complete pinned DXUT IME definitions and complete original storage."""
from pathlib import Path
from prepare_checkpoint32 import definition
from verify import ROOT

def prepare():
 source=(ROOT/'vendor/upstream/saco/d3d9/common/DXUTgui.cpp').read_bytes().decode('latin1')
 out=ROOT/'client/saco'
 macros=source[source.index('#define CHT_IMEFILENAME1'):source.index('// Function pointers',source.index('#define CHT_IMEFILENAME1'))]
 macros+='#define LCID_INVARIANT MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT)\n'
 names=['GetImeId','CheckInputLocale','CheckToggleState','SetupImeApi','ResetCompositionString','StaticMsgProc']
 text='// Complete pinned DXUT IME methods, with reviewed R5 adaptations.\n#include "main.h"\n#include <strsafe.h>\nextern CGame *pGame;\n'+macros+'\n'.join(definition(source,'CDXUTIMEEditBox::'+n) for n in names)+'\n'
 text=text.replace('s_CompString.SetText( "" );','s_CompString.SetText( L"" );')
 text=text.replace('tolower( wszLang[1] )','towlower( wszLang[1] )').replace('tolower( wszDesc[1] )','towlower( wszDesc[1] )').replace('DXUTGetHWND()','pGame->GetMainWindowHwnd()')
 (out/'closure_gui_ime.cpp').write_bytes(text.encode('latin1'))
 callbacks=source[source.index('INPUTCONTEXT* (WINAPI * CDXUTIMEEditBox::_ImmLockIMC)'):source.index('\nDWORD dwImeWaitTick;')]
 indicators=source[source.index('WCHAR     CDXUTIMEEditBox::s_aszIndicator'):source.index('\nbool      CDXUTIMEEditBox::s_bInsertOnType;')]
 minimal='// Complete original DXUT static objects; no unrelated client globals.\n#include "d3d9/common/dxstdafx.h"\n'
 (out/'closure_ime_callbacks.cpp').write_bytes((minimal+callbacks+'\n'+indicators+'\n').encode('latin1'))
 symbols=['s_hklCurrent','s_bVerticalCand','s_bInsertOnType','s_hDllIme','s_ImeState','s_bEnableImeSystem','s_nCompCaret','s_CompString','s_abCompStringAttr','s_bChineseIME','s_Locale']
 for name in symbols:
  line=next(l for l in source.splitlines() if 'CDXUTIMEEditBox::'+name in l and not l.lstrip().startswith('//'))
  assert ';' in line and not line.lstrip().startswith(('void ','bool CDXUTIMEEditBox::'))
  (out/('closure_ime_store_'+name+'.cpp')).write_bytes((minimal+line+'\n').encode('latin1'))
 for name,needle in [('hidden_caret','bool CDXUTEditBox::s_bHideCaret;'),('uniscribe_handle','HINSTANCE CUniBuffer::s_hDll = NULL;')]:
  line=next(l for l in source.splitlines() if l.startswith(needle));(out/('closure_ime_store_'+name+'.cpp')).write_bytes((minimal+line+'\n').encode('latin1'))
 macros=source[source.index('#define UNISCRIBE_DLLNAME'):source.index('#define PLACEHOLDERPROC')]
 text='// Complete original Uniscribe initialization and buffer lifetime methods.\n#include "main.h"\n#include <strsafe.h>\n'+macros+'\n'+'\n'.join(definition(source,'CUniBuffer::'+n) for n in ['Initialize','CUniBuffer','~CUniBuffer'])+'\n'
 (out/'closure_gui_unibuffer_lifetime.cpp').write_bytes(text.encode('latin1'))
if __name__=='__main__':prepare()
