// Alternative only after recording normal-C++ probe; source-derived native-call assembly.
// Uses the same symbolic argument/return-local idiom as base game/rwstuff.cpp.
#include <windows.h>
extern int iGtaVersion;
void *OpenCustomModelStream(int streamType,int access,void *data)
{
    void *result=NULL;
    DWORD nativeAddress=(iGtaVersion!=1)?0x7ECF30:0x7ECEF0;
    _asm push data
    _asm push access
    _asm push streamType
    _asm mov eax,nativeAddress
    _asm call eax
    _asm mov result,eax
    _asm pop eax
    _asm pop eax
    _asm pop eax
    return result;
}
int CloseCustomModelStream(void *stream,void *data)
{
    int result=0;
    DWORD nativeAddress=(iGtaVersion!=1)?0x7ECE60:0x7ECE20;
    _asm push data
    _asm push stream
    _asm mov eax,nativeAddress
    _asm call eax
    _asm mov result,eax
    _asm pop eax
    _asm pop eax
    return result;
}
int ReadCustomModelStream(void *stream,int model)
{
    int result;
    _asm push model
    _asm push stream
    _asm mov edx,0x5371F0
    _asm call edx
    _asm movzx edx,al
    _asm mov result,edx
    _asm pop edx
    _asm pop edx
    return result;
}
