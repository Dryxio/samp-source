"""Restore original complete DXUT translation-unit context with reviewed adaptations."""
import re
from verify import ROOT
from prepare_dxut_state import body
from prepare_checkpoint32 import definition
from prepare_dxut_multimon import prepare as prepare_multimon

def prepare():
 s=(ROOT/'vendor/upstream/saco/d3d9/common/DXUT.cpp').read_text()
 # Complete source bodies; no binary inputs.
 match=re.search(r'(?m)^HRESULT DXUTChangeDevice\([^;{}]*\)\s*\{',s)
 original=definition(s[match.start():],'DXUTChangeDevice',0);s=s.replace(original,body(s,'DXUTChangeDevice'))
 old='int nBitDepthDelta = abs( (long) DXUTColorChannelBits(pDeviceSettingsCombo->AdapterFormat) -\n                                  (long) DXUTColorChannelBits(pOptimalDeviceSettings->AdapterFormat) );'
 assert s.count(old)==1
 s=s.replace(old,'long nAdapterBits = (long) DXUTColorChannelBits(pDeviceSettingsCombo->AdapterFormat);\n        int nBitDepthDelta = abs( nAdapterBits - (long) DXUTColorChannelBits(pOptimalDeviceSettings->AdapterFormat) );')
 # Original callback installation/default processing target the Unicode APIs.
 s=s.replace('SetWindowLongPtr(', 'SetWindowLongPtrW(').replace('DefWindowProc(', 'DefWindowProcW(')
 prepare_multimon()
 monitor=(ROOT/'client/saco/closure_dxut_multimon.cpp').read_text()
 monitor=monitor[monitor.index('//============================================================================='):]
 s=s.replace('#include "dxstdafx.h"','#include "d3d9/common/dxstdafx.h"')
 s=s.replace('#include <multimon.h>',monitor+'\n#undef GetSystemMetrics')
 (ROOT/'client/saco/closure_dxut_state.cpp').write_text('// Full original DXUT context; reviewed R5 adaptations retained.\n'+s)
if __name__=='__main__':prepare()
