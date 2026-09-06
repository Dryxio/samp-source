// Whole R5 actions on an existing scoreboard. No allocation of views/list items.
#include "main.h"
extern CGame *pGame;
extern CNetGame *pNetGame;
extern const int r5RpcClickPlayer;
class R5ScoreboardActionsView
{
public:
 BOOL enabled;
 int reserved4;
 float left,top,scale,width,height,headerHeight;
 int reserved20;
 float column1,column2,column3;
 IDirect3DDevice9 *device;
 CDXUTDialog *dialog;
 CDXUTListBox *listbox;
 int reserved3C,reserved40;
 void GetRect(RECT *rect);
 void Hide(bool restoreInputs);
 void ClickSelected();
};
void R5ScoreboardActionsView::GetRect(RECT *rect)
{
 rect->left=(int)left;
 rect->right=rect->left+(int)width;
 rect->top=(int)top;
 rect->bottom=rect->top+(int)height;
}
void R5ScoreboardActionsView::Hide(bool restoreInputs)
{
 if(enabled && dialog)
 {
  dialog->SetVisible(false);
  listbox->SetEnabled(false);
  listbox->SetVisible(false);
  if(restoreInputs) pGame->ToggleKeyInputsDisabled(0,FALSE);
  enabled=FALSE;
 }
}
void R5ScoreboardActionsView::ClickSelected()
{
 if(enabled && dialog)
 {
  CDXUTListBox *selectedList=listbox;
  DXUTListBoxItem *item=selectedList->GetItem(selectedList->GetSelectedIndex());
  if(item)
  {
   // Only the actual inline text beginning at item offset0 is used.
   // No claim about the rest of R5's larger item layout.
   int playerId=atoi((char*)item);
   RakNet::BitStream stream;
   stream.Write<WORD>((WORD)playerId);
   stream.Write<BYTE>(0);
   pNetGame->GetRakClient()->RPC((char*)&r5RpcClickPlayer,&stream,HIGH_PRIORITY,RELIABLE_ORDERED,0,FALSE);
  }
  Hide(true);
 }
}
