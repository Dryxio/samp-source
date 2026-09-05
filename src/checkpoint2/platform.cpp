// RVA AA4C0. Ordinary C++ wrapper, no original-code dependency.
extern "C" __declspec(dllimport) int __stdcall VirtualProtect(void*,unsigned long,unsigned long,unsigned long*);
extern "C" void __cdecl Unprotect(void* address,unsigned long bytes) {
    unsigned long previous;
    VirtualProtect(address,bytes,0x40,&previous);
}
