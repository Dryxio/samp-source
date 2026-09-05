"""Extract complete resource-cache lifetime and device notification source."""
from prepare_checkpoint32 import definition
from verify import ROOT
NAMES=['DXUTGetGlobalResourceCache','CDXUTResourceCache::~CDXUTResourceCache','CDXUTResourceCache::OnCreateDevice','CDXUTResourceCache::OnResetDevice','CDXUTResourceCache::OnLostDevice','CDXUTResourceCache::OnDestroyDevice']
def prepare():
 s=(ROOT/'vendor/upstream/saco/d3d9/common/DXUTmisc.cpp').read_text()
 (ROOT/'client/saco/closure_dxut_cache.cpp').write_text('// Complete original resource-cache lifetime.\n#include "d3d9/common/dxstdafx.h"\n'+'\n'.join(definition(s,n) for n in NAMES)+'\n')
if __name__=='__main__':prepare()
