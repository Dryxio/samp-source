// Standalone diagnostic proposal, symbolic original provider identity required.
#include "main.h"
extern BYTE *__stdcall GetModelInfo(int model);
BOOL __stdcall IsPickupModelValid(int model)
{
 BYTE *info=GetModelInfo(model);
 if(info && (*(DWORD*)info + 0x14)) return TRUE;
 return FALSE;
}
