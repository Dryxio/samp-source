// Explicit complete symbolic exception after two documented normal-C++ +
// native-bridge attempts: only shared failure-block layout remains different.
// R5 B6250/81. Native CBaseModelInfo::CreateInstance(), slot11, no explicit args.
#include "main.h"
#include <stddef.h>
extern CGame *pGame;
BYTE *__stdcall GetModelInfo(int model);
struct R5FactoryModelPrefix { void **vtable; }; // pointer-only, never allocated
__declspec(naked) void *CreateModelRwInstance(int modelId)
{
    enum {
        FrameBytes=12, ResultLocal=4, ModelLocal=8, VtableLocal=12,
        ModelArgument=8,
        ModelVtable=offsetof(R5FactoryModelPrefix,vtable),
        NativeCreateInstanceOffset=11*sizeof(void*)
    };
    __asm {
        push ebp
        mov ebp, esp
        sub esp, FrameBytes
        mov eax, pGame
        test eax, eax
        push esi
        mov dword ptr [ebp-ResultLocal], 0
        je unavailable
        mov esi, dword ptr [ebp+ModelArgument]
        push esi
        call GetModelInfo
        test eax, eax
        je unavailable
        push esi
        call GetModelInfo
        test eax, eax
        mov dword ptr [ebp-ModelLocal], eax
        jne instantiate
    unavailable:
        xor eax, eax
        pop esi
        mov esp, ebp
        pop ebp
        ret
    instantiate:
        mov eax, dword ptr [eax+ModelVtable]
        mov dword ptr [ebp-VtableLocal], eax
        mov edx, dword ptr [ebp-VtableLocal]
        mov ecx, dword ptr [ebp-ModelLocal]
        call dword ptr [edx+NativeCreateInstanceOffset]
        mov dword ptr [ebp-ResultLocal], eax
        mov eax, dword ptr [ebp-ResultLocal]
        pop esi
        mov esp, ebp
        pop ebp
        ret
    }
}
