// Source-derived symbolic native-call bridge; normal __thiscall function pointer rejected by MSVC2003 C4234.
// Same native-call idiom as existing game/textures.cpp, no encoded bytes or artificial provider.
#include <windows.h>
struct R5TextDrawFloatRect { float left,bottom,right,top; };
void CallRwRenderStateSet(int,int);
void DrawTextdrawNativeSprite(DWORD texture,R5TextDrawFloatRect *rect,DWORD *color)
{
    if(texture) {
        CallRwRenderStateSet(9,2);
        _asm push color
        _asm push rect
        _asm mov ecx,texture
        _asm mov edx,0x728350
        _asm call edx
    }
}
