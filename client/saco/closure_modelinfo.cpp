#include "main.h"
// R5 semantic reconstruction of A7A40/B45A0. These providers are not yet
// certified; their code and supporting storage must not be counted as matched.
// Extended table capacity is provisional, inferred from 1825AC..1A25AC.
// Its complete extent still requires independent review before acceptance.
static BYTE *extendedModelInfoStorage[32768];
BYTE **extendedModelInfos = extendedModelInfoStorage;
BOOL useExtendedModelInfos = FALSE;

BYTE *__stdcall GetModelInfo(int model)
{
    if(useExtendedModelInfos) return extendedModelInfos[model];
    if(model < 0 || model > 20000) return NULL;
    return ((BYTE **)0xA9B0C8)[model];
}

int __stdcall ModelInfoLoaded(UINT model)
{
    BYTE *info = GetModelInfo(model);
    return *(int *)(info + 0x1c);
}
