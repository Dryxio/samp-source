// Alternative after normal-C++ probe: same nativeWORD@8 assembly idiom as original GetModelReferenceCount.
#include <windows.h>
BYTE *__stdcall GetModelInfo(int model);
void __stdcall SetCustomModelReferenceCount(int model,int count)
{
    model=(int)GetModelInfo(model);
    if(model) {
        _asm mov edx,model
        _asm mov bx,word ptr count
        _asm mov word ptr [edx+8],bx
    }
}
