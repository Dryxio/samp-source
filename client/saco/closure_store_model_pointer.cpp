// Complete source storage from game/modelinfo.cpp; isolated from unrelated header objects.
#include "game/common.h"
extern BYTE *extendedModelInfoStorage[65535];
BYTE **extendedModelInfos = extendedModelInfoStorage + 32767;
typedef char complete_storage_size[(sizeof(extendedModelInfos)==4)?1:-1];



