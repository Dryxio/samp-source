"""Extract complete original device-settings selection and resolution logic."""
from prepare_checkpoint32 import definition
from verify import ROOT
NAMES=['DXUTFindValidResolution','DXUTBuildOptimalDeviceSettings','DXUTBuildValidDeviceSettings','DXUTRankDeviceCombo','DXUTFindValidDeviceSettings']
def prepare():
 s=(ROOT/'vendor/upstream/saco/d3d9/common/DXUT.cpp').read_text()
 # R5 evaluates the adapter-format conversion before the optimal-format call.
 # Sequence that first subtraction explicitly; retain the second block unchanged.
 s=s.replace('int nBitDepthDelta = abs( (long) DXUTColorChannelBits(pDeviceSettingsCombo->AdapterFormat) -\n                                  (long) DXUTColorChannelBits(pOptimalDeviceSettings->AdapterFormat) );',
             'long nAdapterBits = (long) DXUTColorChannelBits(pDeviceSettingsCombo->AdapterFormat);\n        int nBitDepthDelta = abs( nAdapterBits - (long) DXUTColorChannelBits(pOptimalDeviceSettings->AdapterFormat) );')
 header='// Complete pinned device settings definitions.\n#include "d3d9/common/dxstdafx.h"\nUINT DXUTColorChannelBits(D3DFORMAT);\nUINT DXUTDepthBits(D3DFORMAT);\nUINT DXUTStencilBits(D3DFORMAT);\nCD3DEnumeration* DXUTPrepareEnumerationObject(bool);\nbool DXUTDoesDeviceComboMatchPreserveOptions(CD3DEnumDeviceSettingsCombo*, DXUTDeviceSettings*, DXUTMatchOptions*);\n'
 (ROOT/'client/saco/closure_dxut_settings.cpp').write_text(header+'\n'.join(definition(s,n) for n in NAMES)+'\n')
if __name__=='__main__':prepare()
