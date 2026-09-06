#include "d3d9/common/dxstdafx.h"
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
#pragma pack(pop)
HRESULT CDXUTListBox::InsertItem(int index,const TCHAR *text,void *data,DWORD color) {
 DXUTListBoxItem *newItem=(DXUTListBoxItem*)new R5CompleteListBoxItem;
 if(!newItem) return E_OUTOFMEMORY;
 StringCchCopyA(((R5CompleteListBoxItem*)newItem)->text,256,text);
 ((R5CompleteListBoxItem*)newItem)->data=data;
 SetRect(&((R5CompleteListBoxItem*)newItem)->activeRect,0,0,0,0);
 ((R5CompleteListBoxItem*)newItem)->selected=false;
 ((R5CompleteListBoxItem*)newItem)->color=color;
 HRESULT result=m_Items.Insert(index,newItem);
 if(SUCCEEDED(result)) m_ScrollBar.SetTrackRange(0,m_Items.GetSize());
 else delete (R5CompleteListBoxItem*)newItem;
 return result;
}
