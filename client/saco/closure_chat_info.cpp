// Normal C++; true info path68070, not debug path680F0.
#include "main.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
// Pointer-only original chat info-color field; no allocation/class size claim.
class R5ChatInfoView {
    BYTE reserved0[0x126];
    DWORD infoColor;
public:
    void AddInfoMessage(char *format,...);
};

void R5ChatInfoView::AddInfoMessage(char *format,...)
{
    char buffer[512];
    memset(buffer,0,sizeof(buffer));
    va_list args;
    va_start(args,format);
    vsprintf(buffer,format,args);
    va_end(args);
    for(char *p=buffer;*p;++p) {
        if(*p>0 && *p<32) *p=' ';
    }
    ((CChatWindow*)this)->AddEntry(4,buffer,NULL,infoColor,0);
}
