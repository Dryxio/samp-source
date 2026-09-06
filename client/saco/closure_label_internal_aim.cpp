#include "main.h"
extern CAMERA_AIM *pcaInternalAim;
CAMERA_AIM *__stdcall GameGetInternalAim() { return pcaInternalAim; }
