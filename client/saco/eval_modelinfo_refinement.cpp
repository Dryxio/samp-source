// Isolated C++ source-form probes for the unmatched R5 B45A0 helper.
#include <windows.h>
extern BYTE *__stdcall GetModelInfo(int model);

int __stdcall ModelInfoLocalVolatile(UINT model)
{
    BYTE * volatile info = GetModelInfo(model);
    volatile int result = 0;
    result = *(int *)(info + 0x1c);
    return result;
}

int __stdcall ModelInfoParameterVolatile(volatile UINT model)
{
    BYTE * volatile info = GetModelInfo(model);
    model = 0;
    model = *(int *)(info + 0x1c);
    return model;
}

int __stdcall ModelInfoPointerVolatile(UINT model)
{
    BYTE * volatile info = GetModelInfo(model);
    int result = 0;
    result = *(int *)(info + 0x1c);
    return result;
}
