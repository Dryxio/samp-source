// Partial R5 pointer views; never allocate or index whole objects.
#include <windows.h>
struct R5CustomModelEntryView {
    unsigned char unknown0[7];
    BYTE type;
    unsigned char unknown8[8];
    int modelId;
    unsigned char unknown14[0x36];
    DWORD textureId;
    unsigned char unknown4e[10];
    bool ready;
};
class R5CustomModelManagerView {
    R5CustomModelEntryView **items;
    unsigned int count;
    R5CustomModelEntryView *GetAt(unsigned int index) { return index<count ? items[index] : 0; }
    R5CustomModelEntryView *GetAtChecked(unsigned int index) { if(index>=count) return 0; return items[index]; }
public:
    bool HasTextureModel(int id);
    bool IsModelReady(int id);
    DWORD GetTextureId(int id);
};
// Original D150 /58
bool R5CustomModelManagerView::HasTextureModel(int id)
{
    unsigned int x=0;
    while(x!=count) {
        R5CustomModelEntryView *entry=GetAtChecked(x);
        if(entry->modelId==id && entry->type==2) return true;
        x++;
    }
    return false;
}
// Original D190 /53
bool R5CustomModelManagerView::IsModelReady(int id)
{
    unsigned int x=0;
    while(x!=count) {
        R5CustomModelEntryView *entry=GetAtChecked(x);
        if(entry->modelId==id) return entry->ready;
        x++;
    }
    return false;
}
// Original D280 /59
DWORD R5CustomModelManagerView::GetTextureId(int id)
{
    unsigned int x=0;
    while(x!=count) {
        R5CustomModelEntryView *entry=GetAt(x);
        if(entry && entry->modelId==id && entry->type==2) return entry->textureId;
        x++;
    }
    return 0;
}
