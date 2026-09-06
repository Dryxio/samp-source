// Existing accepted stream/native bridge style; no execution/deployment.
#include <windows.h>
extern int iGtaVersion;
extern BYTE r5CustomModelLoading;
void UnFuck(DWORD,int);
void *OpenCustomModelStream(int,int,void*);
int CloseCustomModelStream(void*,void*);
int FindCustomModelStreamChunk(void *stream,unsigned chunkType,unsigned *chunkLength,unsigned *version)
{
    int result=0;
    DWORD function=(iGtaVersion!=1)?0x7ED310:0x7ED2D0;
    __asm {
        push version
        push chunkLength
        push chunkType
        push stream
        mov eax, function
        call eax
        mov result, eax
        pop eax
        pop eax
        pop eax
        pop eax
    }
    return result;
}
void *ReadCustomPedClumpFromStream(void *stream)
{
    void *result=0;
    DWORD function=(iGtaVersion!=1)?0x74B470:0x74B420;
    __asm {
        push stream
        mov eax, function
        call eax
        mov result, eax
        pop eax
    }
    return result;
}
void *ReadPedModelClump(char *path)
{
    void *stream=OpenCustomModelStream(2,1,path);
    if(!stream) return NULL;
    UnFuck(0x41B1D0,1);
    *(BYTE*)0x41B1D0=0xC3;
    r5CustomModelLoading=1;
    if(FindCustomModelStreamChunk(stream,16,0,0)) {
        void *clump=ReadCustomPedClumpFromStream(stream);
        CloseCustomModelStream(stream,0);
        *(BYTE*)0x41B1D0=0x64;
        r5CustomModelLoading=0;
        return clump;
    }
    CloseCustomModelStream(stream,0);
    *(BYTE*)0x41B1D0=0x64;
    r5CustomModelLoading=0;
    return NULL;
}
