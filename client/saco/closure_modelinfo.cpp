// Generated complete definitions from game/modelinfo.cpp; see tools/prepare_actor_closure.py.
#include <time.h>
#include <math.h>
#include "main.h"
#include "game/util.h"
extern BYTE **extendedModelInfos;
extern BOOL useExtendedModelInfos;


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
