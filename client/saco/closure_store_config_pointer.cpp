// Complete source storage from main.cpp; isolated from unrelated header objects.
#include "game/common.h"
#include "config.h"
CConfig *pConfig=0;
typedef char complete_storage_size[(sizeof(pConfig)==4)?1:-1];



