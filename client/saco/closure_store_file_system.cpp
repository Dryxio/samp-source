// Complete source storage from main.cpp; isolated from unrelated header objects.
#include "game/common.h"
class CFileSystem;
CFileSystem *pFileSystem=0;
typedef char complete_storage_size[(sizeof(pFileSystem)==4)?1:-1];



