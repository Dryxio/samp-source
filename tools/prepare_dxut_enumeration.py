"""Extract complete enumeration lifecycle/list-building definitions."""
from prepare_checkpoint32 import definition
from verify import ROOT
NAMES=['CD3DEnumeration::'+n for n in ['CD3DEnumeration','~CD3DEnumeration','ClearAdapterInfoList','EnumerateDevices','EnumerateDeviceCombos','BuildDepthStencilFormatList','BuildMultiSampleTypeList','BuildDSMSConflictList','BuildPresentIntervalList','ResetPossibleDepthStencilFormats','ResetPossibleMultisampleTypeList','ResetPossiblePresentIntervalList','GetAdapterInfo','GetDeviceInfo','GetDeviceSettingsCombo','SetPossibleVertexProcessingList','GetPossibleVertexProcessingList']]+['CD3DEnumAdapterInfo::~CD3DEnumAdapterInfo','CD3DEnumDeviceInfo::~CD3DEnumDeviceInfo']
def prepare():
 s=(ROOT/'vendor/upstream/saco/d3d9/common/DXUTenum.cpp').read_text()
 (ROOT/'client/saco/closure_dxut_enumeration.cpp').write_text('// Complete pinned DXUT enumeration methods.\n#include "d3d9/common/dxstdafx.h"\n'+'\n'.join(definition(s,n) for n in NAMES)+'\n')
if __name__=='__main__':prepare()
