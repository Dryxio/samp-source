// B1FC0/127 whole C++ candidate. Never execute in GTA; compile/match only.
#include <windows.h>
void *OpenCustomModelStream(int,int,void*);
int CloseCustomModelStream(void*,void*);
int ReadCustomModelStream(void*,int);
void __stdcall SetCustomModelReferenceCount(int,int);
void UnFuck(DWORD,int);
extern BYTE r5CustomModelLoading;
int LoadCustomModelClump(char *path,int model)
{
    void *stream=OpenCustomModelStream(2,1,path);
    if(!stream) return 0;
    UnFuck(0x41B1D0,1);
    *(BYTE*)0x41B1D0=0xC3;
    r5CustomModelLoading=1;
    if(!ReadCustomModelStream(stream,model)) {
        *(BYTE*)0x41B1D0=0x64;
        r5CustomModelLoading=0;
        return 0;
    }
    CloseCustomModelStream(stream,0);
    SetCustomModelReferenceCount(model,0);
    *(BYTE*)0x41B1D0=0x64;
    r5CustomModelLoading=0;
    return 1;
}
