// R5 0x71B30/36, attached-object slot view; ordinary C++ and bounded indexing.
#include "main.h"
#pragma pack(push,1)
struct R5AttachedObjectSlot { unsigned char data[0x34]; };
class R5AttachedObjectSlotsView {
    unsigned char prefix[0x4c];
    int active[10];
    R5AttachedObjectSlot slots[10];
public:
    R5AttachedObjectSlot *GetSlot(int index);
};
#pragma pack(pop)
R5AttachedObjectSlot *R5AttachedObjectSlotsView::GetSlot(int index)
{
    if (index < 0 || index >= 10) return NULL;
    if (!active[index]) return NULL;
    return &slots[index];
}
