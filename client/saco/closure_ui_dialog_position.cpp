// R5 0x71820/95: position the original bottom-right application dialog.
#include "main.h"
#pragma pack(push,1)
struct R5ApplicationDialogGeometry {
    unsigned char prefix[0x116];
    int x, y, width, height;
    void SetSize(int w, int h) { width=w; height=h; }
    void SetLocation(int px, int py) { x=px; y=py; }
};
class R5ApplicationDialogPositionView {
    int field0;
    int x, y, width, height;
    unsigned char gap14[0xc];
    R5ApplicationDialogGeometry *dialog;
public:
    void PositionDialog();
};
#pragma pack(pop)
void R5ApplicationDialogPositionView::PositionDialog()
{
    dialog->SetSize(width, height);
    RECT clientRect;
    GetClientRect(*(HWND *)0xC97C1C, &clientRect);
    x = clientRect.right - width - 5;
    y = clientRect.bottom - height - 10;
    dialog->SetLocation(x, y);
}
