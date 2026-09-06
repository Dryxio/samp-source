#include "main.h"
class R5ChatGameMenuView { public: BOOL IsMenuActive(); };
BOOL R5ChatGameMenuView::IsMenuActive()
{
 const DWORD NativeFrontendMenuActive=0xBA67A4;
 if(*(DWORD*)NativeFrontendMenuActive) return TRUE;
 return FALSE;
}
