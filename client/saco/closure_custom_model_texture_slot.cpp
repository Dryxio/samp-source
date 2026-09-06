// B4660/29 proposal: C++ with source-derived symbolic field-access assembly.
// Normal C++ attempts: 22 bytes, then29withwrongloadorder, then29withwrongregisters.
// Mirrors existing game/util.cpp GetModelReferenceCount's native model-field access idiom.
// No naked function, emitted bytes, address masking or copied machine-code payload.
#include <windows.h>
BYTE *__stdcall GetModelInfo(int model);
void __stdcall SetCustomModelTextureSlot(int model,int slot)
{
    model=(int)GetModelInfo(model);
    _asm mov edx, model
    _asm mov eax, slot
    _asm mov word ptr [edx+10], ax
}
