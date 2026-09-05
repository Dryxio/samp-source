// Whole B44E0/58 C++ candidate. Native vtable identities from all5 comparisons.
#include <windows.h>
BOOL __stdcall IsSupportedCustomModelInfo(BYTE *info)
{
    if(info) {
        DWORD vtable=*(DWORD*)info;
        if(vtable==0x85BBF0 || vtable==0x85BC30 || vtable==0x85BC70 ||
           vtable==0x85BCB0 || vtable==0x85BCF0) return TRUE;
    }
    return FALSE;
}
