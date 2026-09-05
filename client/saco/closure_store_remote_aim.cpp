// Complete source storage from game/aimstuff.cpp; isolated from unrelated header objects.
#include "game/common.h"
#include "game/aimstuff.h"
CAMERA_AIM caRemotePlayerAim[PLAYER_PED_SLOTS];
typedef char complete_storage_size[(sizeof(caRemotePlayerAim)==10080)?1:-1];



