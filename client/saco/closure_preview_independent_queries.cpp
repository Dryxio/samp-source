// Independent prerequisites only; no preview-renderer coverage claimed.
#include <windows.h>
extern BYTE *__stdcall GetModelInfo(int model);
BOOL __stdcall R5PreviewModelInfoPresent(int model)
{
    return GetModelInfo(model)!=0;
}
struct R5PreviewTypeOneEntryView {
    BYTE prefix[7]; BYTE type; BYTE unknown8[8]; int modelId;
};
class R5PreviewTypeOneManagerView {
    R5PreviewTypeOneEntryView **items;
    unsigned int count;
    R5PreviewTypeOneEntryView *GetAtChecked(unsigned int i) { if(i>=count)return 0;return items[i]; }
public:
    bool HasModel(int id);
};
bool R5PreviewTypeOneManagerView::HasModel(int id)
{
    unsigned int i=0;
    while(i!=count) {
        R5PreviewTypeOneEntryView *entry=GetAtChecked(i);
        if(entry->modelId==id && entry->type==1)return true;
        i++;
    }
    return false;
}
