#include "common.h"
// R5 A7A00 clears 65535 pointer entries and copies 20000 native entries
// through a pointer to element 32767. Reconstructed from those full accesses.
BYTE *extendedModelInfoStorage[65535];
BYTE **extendedModelInfos = extendedModelInfoStorage + 32767;
BOOL useExtendedModelInfos = FALSE;

BYTE *__stdcall GetModelInfo(int model)
{
    if(useExtendedModelInfos) return extendedModelInfos[model];
    if(model < 0 || model > 20000) return NULL;
    return ((BYTE **)0xA9B0C8)[model];
}

// Unmatched provider: retain its semantic implementation, exclude coverage.
int __stdcall ModelInfoLoaded(UINT model)
{
    BYTE *info = GetModelInfo(model);
    return *(int *)(info + 0x1c);
}
