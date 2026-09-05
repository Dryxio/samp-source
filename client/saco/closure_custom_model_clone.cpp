// Whole A7AD0/91 source candidate, no fake model storage.
#include <windows.h>
#include <stdlib.h>
#include <string.h>
extern BYTE **extendedModelInfos;
BYTE *__stdcall GetModelInfo(int model);
BOOL __stdcall IsSupportedCustomModelInfo(BYTE *info);
BYTE *CloneCustomModelInfo(int sourceModel, int destinationModel)
{
    BYTE *existing=extendedModelInfos[destinationModel];
    if(existing) return existing;
    BYTE *copy=(BYTE*)calloc(32,1);
    BYTE *info=GetModelInfo(sourceModel);
    if(info && IsSupportedCustomModelInfo(info)) {
        memcpy(copy,info,32);
        extendedModelInfos[destinationModel]=copy;
        return copy;
    }
    return NULL;
}
