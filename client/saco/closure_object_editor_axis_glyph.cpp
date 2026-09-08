// R5 0x71FA0, whole146-byte candidate. Typed field view, no assembly.
#include "main.h"
class R5ObjectEditorAxisGlyphView {
    unsigned char prefix[0x7c];
    int mode;
public:
    const char *GetAxisGlyph(int axis);
};
const char *R5ObjectEditorAxisGlyphView::GetAxisGlyph(int axis)
{
    if (axis == 0) {
        if (mode == 0) return "6";
        if (mode == 1) return "3";
        if (mode == 2) return "9";
    } else if (axis == 1) {
        if (mode == 0) return "7";
        if (mode == 1) return "4";
        if (mode == 2) return "A";
    } else if (axis == 2) {
        if (mode == 0) return "8";
        if (mode == 1) return "5";
        if (mode == 2) return "B";
    }
    return "0";
}
