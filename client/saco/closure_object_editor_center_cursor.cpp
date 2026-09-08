// R5 editor cursor centering, complete function723c0/81; pointer view only.
#include "main.h"
#pragma pack(push,1)
class R5ObjectEditorCursorView {
 BYTE unknown[0xa7]; int cursorX; int cursorY;
public: void CenterCursor();
};
#pragma pack(pop)
void R5ObjectEditorCursorView::CenterCursor() {
 RECT rect;
 GetClientRect(*(HWND*)0xC97C1C,&rect);
 int x=(rect.right-rect.left)/2;
 int y=(rect.bottom-rect.top)/2;
 cursorX=x;
 cursorY=y;
 SetCursorPos(x,y);
}
