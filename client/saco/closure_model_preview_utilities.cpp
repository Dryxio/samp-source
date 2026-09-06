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

enum { NativeCollisionPointer=offsetof(R5NativeModelCollisionView,collision) };

// Explicit three-instruction field-copy exception after two normal C++ trials.
// B6360/74: two observed local spills retained explicitly; branch exits first.
float GetModelCollisionRadius(int model)
{
    if(!GetModelInfo(model)) return 0.0f;
    R5NativeModelCollisionView *volatile info=(R5NativeModelCollisionView*)GetModelInfo(model);
    R5ModelCollisionBoundsView *volatile collision=0;
    __asm {
        mov eax, info
        mov edx, dword ptr [eax+NativeCollisionPointer]
        mov collision, edx
    }
    R5ModelCollisionBoundsView *value=collision;
    if(!value) return 0.0f;
    return value->radius;
}
// B6410/93: same original pair of stack-backed locals.
BOOL GetModelCollisionCentre(int model,VECTOR *output)
{
    if(!GetModelInfo(model)) return FALSE;
    R5NativeModelCollisionView *volatile info=(R5NativeModelCollisionView*)GetModelInfo(model);
    R5ModelCollisionBoundsView *volatile collision=0;
    __asm {
        mov eax, info
        mov edx, dword ptr [eax+NativeCollisionPointer]
        mov collision, edx
    }
    R5ModelCollisionBoundsView *value=collision;
    if(!value) return FALSE;
    output->X=value->centre.X;
    output->Y=value->centre.Y;
    output->Z=value->centre.Z;
    return TRUE;
}
// B4520/9; actual callers pass GetModelInfo result. No vtable ownership claim.
void **__stdcall GetNativeModelVtable(void *modelInfo)
{
    return *(void***)modelInfo;
}
// B1430/20: callback query, type1 RpAtomic only.
R5AtomicRenderCallback GetAtomicRenderCallback(RpAtomic *atomic)
{
    if(atomic && ((R5RwObjectFrameView*)atomic)->type==1)
        return ((R5RwAtomicRenderView*)atomic)->renderCallback;
    return NULL;
}
