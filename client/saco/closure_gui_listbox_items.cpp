// Whole R5 ListBox item allocations; container itself remains a pointer-only view.
#include "main.h"
#include <stddef.h>
#pragma pack(push,1)
struct R5CompleteListBoxItem
{
 char text[257];
 char columns[3][129];
 void *data;
 RECT activeRect;
 bool selected;
 DWORD color;
 bool marked;
};
typedef char VerifyR5ListItemSize[(sizeof(R5CompleteListBoxItem)==0x29E)?1:-1];
typedef char VerifyR5ListItemData[(offsetof(R5CompleteListBoxItem,data)==0x284)?1:-1];
typedef char VerifyR5ListItemColor[(offsetof(R5CompleteListBoxItem,color)==0x299)?1:-1];
class R5ListBoxItemOwnerView
{
public:
 BYTE prefix[0x4D];
 int columnCount;
 BYTE gap51[0x14C-0x51];
 CGrowableArray<DXUTListBoxItem*> items;
 HRESULT AddItem(const char *text,void *data,DWORD color);
 void SetColumnText(int item,int column,const char *text);
};
#pragma pack(pop)
HRESULT R5ListBoxItemOwnerView::AddItem(const char *text,void *data,DWORD color)
{
 DXUTListBoxItem *newItem=(DXUTListBoxItem*)new R5CompleteListBoxItem;
 if(!newItem) return E_OUTOFMEMORY;
 StringCchCopyA(((R5CompleteListBoxItem*)newItem)->text,256,text);
 ((R5CompleteListBoxItem*)newItem)->data=data;
 SetRect(&((R5CompleteListBoxItem*)newItem)->activeRect,0,0,0,0);
 ((R5CompleteListBoxItem*)newItem)->selected=false;
 ((R5CompleteListBoxItem*)newItem)->color=color;
 ((R5CompleteListBoxItem*)newItem)->marked=false;
 for(int i=0;i<3;i++) memset(((R5CompleteListBoxItem*)newItem)->columns[i],0,64);
 // Actual pre-existing pointer-array specialization; element zero-offset cast only.
 HRESULT result=items.Add(newItem);
 if(FAILED(result)) delete (R5CompleteListBoxItem*)newItem;
 else ((CDXUTScrollBar*)((BYTE*)this+0x5D))->SetTrackRange(0,items.GetSize());
 return result;
}
void R5ListBoxItemOwnerView::SetColumnText(int item,int column,const char *text)
{
 if(item<0 || item>=items.GetSize() || column<0 || column>=columnCount) return;
 memset(((R5CompleteListBoxItem*)items[item])->columns[column],0,128);
 strncpy(((R5CompleteListBoxItem*)items[item])->columns[column],text,128);
}
// Instantiate real header source; no wrapper/alias substituted for its Add method.
template HRESULT CGrowableArray<DXUTListBoxItem*>::Add(DXUTListBoxItem *const&);
