// Generated complete definitions from game/modelinfo.cpp; see tools/prepare_actor_closure.py.
#include <time.h>
#include <math.h>
#include "main.h"
#include "game/util.h"
extern BYTE **extendedModelInfos;
extern BOOL useExtendedModelInfos;


BYTE *__stdcall GetModelInfo(int model);

int __stdcall ModelInfoLoaded(UINT model)
{
    BYTE *info = GetModelInfo(model);
    return *(int *)(info + 0x1c);
}
