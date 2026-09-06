// Complete R5 B45A0/40. Ordinary C++16 then volatile C++40 failed:
// volatile form selects ECX instead of original EAX for the native info load.
// Bounded three-instruction symbolic bridge, same actual field and providers.
#include <windows.h>
BYTE *__stdcall GetModelInfo(int);
int __stdcall ModelInfoLoaded(UINT model)
{
    BYTE *info=GetModelInfo(model);
    model=0;
    enum { NativeRwObjectMember=0x1C };
    __asm {
        mov eax, info
        mov edx, dword ptr [eax+NativeRwObjectMember]
        mov model, edx
    }
    return model;
}
