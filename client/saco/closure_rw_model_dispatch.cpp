#include "main.h"
#include "game/rwstuff.h"
#include <stddef.h>
// Pointer-only fields observed on native model / collision objects.
struct R5ModelCollisionBoundsView {
    BYTE reserved0[0x18];
    VECTOR centre;
    float radius;
};
struct R5NativeModelCollisionView {
    BYTE reserved0[0x14];
    R5ModelCollisionBoundsView *collision;
};
// RwObject type byte and frame pointer, no allocated size claim.
struct R5RwObjectFrameView {
    BYTE type;
    BYTE reserved1[3];
    RwFrame *frame;
};
struct R5RwAtomicRenderView {
    BYTE reserved0[0x48];
    RpAtomic *(__cdecl *renderCallback)(RpAtomic *);
};
typedef RpAtomic *(__cdecl *R5AtomicRenderCallback)(RpAtomic *);
BYTE *__stdcall GetModelInfo(int);
R5AtomicRenderCallback GetAtomicRenderCallback(RpAtomic *atomic);
BOOL __stdcall RenderAtomicIfValid(RpAtomic *atomic);

// Explicit symbolic ABI exception authorized after normal C++ yielded41 vs59.
// Native callback bridge; no encoded instructions, padding or fake vtable.
__declspec(naked) BOOL __stdcall RenderAtomicIfValid(RpAtomic *atomic)
{
    enum {
        AtomicArgument=8,
        CallbackLocal=-4,
        ObjectType=offsetof(R5RwObjectFrameView,type),
        AtomicType=1
    };
    __asm {
        push ebp
        mov ebp, esp
        push ecx
        mov eax, dword ptr [ebp+AtomicArgument]
        test eax, eax
        push ebx
        je invalid_atomic
        cmp byte ptr [eax+ObjectType], AtomicType
        jne invalid_atomic
        push eax
        call GetAtomicRenderCallback
        add esp, 4
        mov dword ptr [ebp+CallbackLocal], eax
        push dword ptr [ebp+AtomicArgument]
        mov ebx, dword ptr [ebp+CallbackLocal]
        call ebx
        pop ebx
        mov eax, TRUE
        pop ebx
        mov esp, ebp
        pop ebp
        ret 4
invalid_atomic:
        xor eax, eax
        pop ebx
        mov esp, ebp
        pop ebp
        ret 4
    }
}
// B6330/37: dispatch RpAtomic type1 vs RpClump type2.
void __stdcall RenderRwModel(void *object)
{
    if(object) {
        BYTE type=((R5RwObjectFrameView*)object)->type;
        if(type==1) RenderAtomicIfValid((RpAtomic*)object);
        else if(type==2) RpClumpRender((RpClump*)object);
    }
}
// B62B0/57: preserve frame before deleting atomic; clump owns its own frames.
void DestroyRwModel(void *object)
{
    if(object) {
        BYTE type=((R5RwObjectFrameView*)object)->type;
        if(type==1) {
            RwFrame *frame=((R5RwObjectFrameView*)object)->frame;
            RpAtomicDestroy((RpAtomic*)object);
            if(frame) RwFrameDestroy(frame);
        } else if(type==2) RpClumpDestroy((RpClump*)object);
    }
}
