// Complete R5 actor-model validation helper; no native class-layout claim.
#include <windows.h>
extern BYTE *__stdcall GetModelInfo(int model);
BOOL __stdcall IsActorPedModelValid(int model)
{
 if(model<0 || model>30000)return FALSE;
 BYTE *info=GetModelInfo(model);
 if(info && *(DWORD*)info==0x85BDC0)return TRUE;
 return FALSE;
}
