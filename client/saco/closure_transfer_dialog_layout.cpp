#include "main.h"
#include <stddef.h>
extern CFontRender *pDefaultFont;
#pragma pack(push,1)
class R5TransferDialogLayoutView
{
public:
 BYTE prefix[0x0c];
 int left,top,width,height;
 BYTE unknown1C[8];
 int contentHeight;
 CDXUTDialog *dialog;
 CDXUTListBox *list;
 void UpdateLayout();
};
#pragma pack(pop)
typedef char R5TransferDialogOffset[offsetof(R5TransferDialogLayoutView,dialog)==0x28 ? 1 : -1];
typedef char R5TransferListOffset[offsetof(R5TransferDialogLayoutView,list)==0x2c ? 1 : -1];
void R5TransferDialogLayoutView::UpdateLayout()
{
 dialog->SetSize(width,height);
 RECT client;
 GetClientRect(*(HWND*)0xC97C1C,&client);
 left=client.right/2-width/2;
 top=client.bottom/2-height/2;
 dialog->SetLocation(left,top);
 contentHeight=height-dialog->GetCaptionHeight();
 SIZE text=pDefaultFont->MeasureText2("Y");
 list->SetLocation(2,text.cy+2);
 list->SetSize(width-4,contentHeight-text.cy-4);
}
