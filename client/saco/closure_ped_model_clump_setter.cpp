// Explicit native CClumpModelInfo::SetClump vtable slot16 bridge.
#include <windows.h>
BYTE *__stdcall GetModelInfo(int);
enum { PedClumpMember=0x1C, NativeSetClumpOffset=16*sizeof(void*) };
void __stdcall SetPedModelClump(int model,void *clump)
{
    model=(int)GetModelInfo(model);
    __asm {
        mov ecx, model
        mov edx, clump
        mov dword ptr [ecx+PedClumpMember], edx
        push clump
        mov edx, [ecx]
        call dword ptr [edx+NativeSetClumpOffset]
    }
}
