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
extern CUnkClass5 *pUnkClass5;
extern CDXUTDialog *r5UiDialog_scoreboard;
extern CDXUTDialog *r5UiDialog_chat_command;
extern CDXUTDialog *r5UiDialog_spawn;
extern CDXUTDialog *r5UiDialog_application3;
extern CDXUTDialog *r5UiDialog_application4;
extern CDXUTDialog *r5UiDialog_transfer;
void RefreshApplicationUiFonts()
{
 if(r5UiDialog_scoreboard) r5UiDialog_scoreboard->UpdateFont();
 if(r5UiDialog_chat_command) r5UiDialog_chat_command->UpdateFont();
 if(r5UiDialog_spawn) r5UiDialog_spawn->UpdateFont();
 if(r5UiDialog_application3) r5UiDialog_application3->UpdateFont();
 if(r5UiDialog_application4) r5UiDialog_application4->UpdateFont();
 if(r5UiDialog_transfer) r5UiDialog_transfer->UpdateFont();
 if(pUnkClass5) ((R5TransferDialogLayoutView*)pUnkClass5)->UpdateLayout();
}
