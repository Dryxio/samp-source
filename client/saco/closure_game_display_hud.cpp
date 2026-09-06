// Whole native HUD state provider A1DB0/40, derived from game/game.cpp.
// Inline the already-known ToggleRadar body without defining that owned symbol twice.
#include "main.h"
static __forceinline void UpdateRadarVisibility(int toggle)
{
    *(PBYTE)0xBAA3FB=(BYTE)!toggle;
}
void CGame::DisplayHud(BOOL display)
{
    if(display) {
        *(BYTE*)ADDR_ENABLE_HUD=1;
        UpdateRadarVisibility(1);
    } else {
        *(BYTE*)ADDR_ENABLE_HUD=0;
        UpdateRadarVisibility(0);
    }
}
