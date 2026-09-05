// Native test executable. Loads only our linked capsule and Windows libraries.
#include <windows.h>
#include <stdio.h>
#include "../../src/checkpoint2/buffer.h"
extern "C" {
__declspec(dllimport) unsigned FormatBits(unsigned);
__declspec(dllimport) void Unprotect(void*,unsigned long);
__declspec(dllimport) void* CpAllocate(unsigned);
__declspec(dllimport) void CpFree(void*);
__declspec(dllimport) void CpDestroyLayer(Layer*);
__declspec(dllimport) void CpClear(Storage*,bool);
__declspec(dllimport) void CpGravity(float);
__declspec(dllimport) int CpVirtualTest();
__declspec(dllimport) int CpUnwind(int);
}
static int checks;
#define CHECK(x) do { ++checks; if (!(x)) { printf("FAIL line %d: %s\n",__LINE__,#x); return 1; } } while (0)
static unsigned ExpectedBits(unsigned v) {
    const unsigned formats[]={20,21,22,23,24,25,26,27,29,30,31,32,35,36};
    const unsigned values[]={8,8,8,5,5,5,4,2,2,4,10,8,10,16};
    for (unsigned i=0;i<14;++i) if(v==formats[i]) return values[i];
    return 0;
}
int main() {
    printf("capsule loaded at %p\n",GetModuleHandleA("capsule.dll"));
    CHECK(GetModuleHandleA("samp.dll")==0);
    for(unsigned i=0;i<1024;++i) CHECK(FormatBits(i)==ExpectedBits(i));
    CHECK(FormatBits(0xffffffff)==0);
    CHECK(FormatBits(0x80000000)==0);
    void* page=VirtualAlloc(0,4096,MEM_RESERVE|MEM_COMMIT,PAGE_READONLY);
    CHECK(page!=0);
    Unprotect(page,4);
    MEMORY_BASIC_INFORMATION mbi;
    CHECK(VirtualQuery(page,&mbi,sizeof(mbi))==sizeof(mbi));
    CHECK(mbi.Protect==PAGE_EXECUTE_READWRITE);
    *(int*)page=0x12345678;
    CHECK(*(int*)page==0x12345678);
    CHECK(VirtualFree(page,0,MEM_RELEASE));
    // Gravity's fixed GTA address is tested in the linked-image emulator.
    // A standalone Windows process need not have that address free.
    const unsigned capacities[]={0,1,32,512,513,1024};
    for(int k=0;k<6;++k) for(int keep=0;keep<2;++keep) {
        Layer* p=(Layer*)CpAllocate(sizeof(Layer));
        unsigned capacity=capacities[k];
        p->data=capacity?CpAllocate(capacity):0;
        p->count=17;p->capacity=capacity;p->active=true;
        void* old=p->data;
        CpClear(p,keep!=0);
        CHECK(p->count==(capacity?0:17));
        CHECK(p->capacity==((capacity>512&&!keep)?0:capacity));
        CHECK(p->data==((capacity>512&&!keep)?0:old));
        CpDestroyLayer(p);
        // Inspect representation before releasing raw storage, after destruction.
        CHECK(((unsigned char*)p)[20]==0);
        CpFree(p);
    }
    CHECK(CpVirtualTest()==1);
    for(int j=0;j<128;++j) { CHECK(CpUnwind(0)==1); CHECK(CpUnwind(1)==1); }
    CHECK(GetModuleHandleA("samp.dll")==0);
    printf("PASS native: %d checks; switch, WinAPI, closed cleanup, virtual dispatch, C++ unwind\n",checks);
    return 0;
}
