// Test capsule glue: these exports are NOT counted as reconstructed R5 code.
#include <windows.h>
#include <new>
#include "buffer.h"
#include "../game.cpp"
#include "virtual.cpp"

VirtualActor::~VirtualActor() {}
static int destroyed;
class TestActor : public VirtualActor {
public: ~TestActor() { ++destroyed; }
};

extern "C" {
__declspec(dllexport) void* CpAllocate(unsigned n) { return ::operator new(n); }
__declspec(dllexport) void CpFree(void* p) { ::operator delete(p); }
__declspec(dllexport) void CpDestroyLayer(Layer* p) { p->~Layer(); }
__declspec(dllexport) void CpClear(Storage* p,bool keep) { p->ClearStorage(keep); }
__declspec(dllexport) void CpGravity(float value) { Game g; g.SetGravity(value); }
__declspec(dllexport) int CpVirtualTest() {
    VirtualPool* p=new VirtualPool;
    ZeroMemory(p,sizeof(*p));
    destroyed=0;
    const int ids[]={0,1,57,999};
    for (int i=0;i<4;++i) {
        p->actors[ids[i]]=new TestActor;
        p->slots[ids[i]]=1;
        p->gtaIds[ids[i]]=123;
    }
    p->lastActive=999;
    if (p->Delete(1000)!=0 || p->Delete(65535)!=0) return -1;
    p->DeleteAll();
    int ok=(destroyed==4 && p->lastActive==0);
    for (int j=0;j<1000;++j)
        if (p->slots[j] || p->actors[j] || p->gtaIds[j]) ok=0;
    delete p;
    return ok;
}
__declspec(dllexport) int CpUnwind(int mode) {
    unsigned before,after;
    __asm {
        mov eax, fs:[0]
        mov before, eax
    }
    int caught=0;
    try {
        Layer layer;
        layer.capacity=mode?1024:32;
        layer.count=17;
        layer.data=::operator new(layer.capacity);
        layer.active=true;
        throw 42;
    } catch (int value) { caught=value; }
    __asm {
        mov eax, fs:[0]
        mov after, eax
    }
    return caught==42 && before==after;
}
BOOL WINAPI DllMain(HINSTANCE,DWORD,LPVOID) { return TRUE; }
}
