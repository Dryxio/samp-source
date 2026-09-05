"""Extract complete pinned dynamic Direct3D loading and API wrappers."""
from prepare_checkpoint32 import definition
from verify import ROOT
NAMES=['DXUT_EnsureD3DAPIs','DXUT_Dynamic_Direct3DCreate9','DXUT_Dynamic_D3DPERF_BeginEvent','DXUT_Dynamic_D3DPERF_EndEvent','DXUT_Dynamic_D3DPERF_SetMarker','DXUT_Dynamic_D3DPERF_SetRegion','DXUT_Dynamic_D3DPERF_QueryRepeatFrame','DXUT_Dynamic_D3DPERF_SetOptions','DXUT_Dynamic_D3DPERF_GetStatus']
def prepare():
 s=(ROOT/'vendor/upstream/saco/d3d9/common/DXUTmisc.cpp').read_text()
 globals=s[s.index('typedef IDirect3D9* (WINAPI * LPDIRECT3DCREATE9)'):s.index('// Ensure function pointers are initialized')]
 text='// Complete original dynamic Direct3D API support.\n#include "d3d9/common/dxstdafx.h"\n#include <strsafe.h>\n'+globals+'\n'+'\n'.join(definition(s,n) for n in NAMES)+'\n'
 (ROOT/'client/saco/closure_dxut_dynamic.cpp').write_text(text)
if __name__=='__main__':prepare()
