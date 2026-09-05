#include <windows.h>
#include <stdio.h>
extern "C" __declspec(dllimport) int FoundationSelfTest();
int main() {
    printf("foundation loaded at %p\n",GetModuleHandleA("foundation.dll"));
    return FoundationSelfTest();
}
