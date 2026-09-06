#include "main.h"
#include <stddef.h>
extern CNetGame *pNetGame;
#pragma pack(push,1)
class R5ScoreboardPlayerQueriesView
{
public:
 BYTE prefix[0x2A];
 BOOL slots[MAX_PLAYERS];
 BYTE middleGap[4016];
 CNetPlayer *players[MAX_PLAYERS];
 BOOL GetSlotState(PLAYERID id) { if(id>=MAX_PLAYERS) return FALSE; return slots[id]; }
 int GetCount(BOOL includeNPC);
 BOOL IsNPC(PLAYERID id);
 int GetScore(PLAYERID id);
 int GetPing(PLAYERID id);
 CRemotePlayer *GetRemote(PLAYERID id);
};
#pragma pack(pop)
typedef char R5ScoreboardPlayerPointerOffset[(offsetof(R5ScoreboardPlayerQueriesView,players)==0x1F8A)?1:-1];
#pragma pack(push,1)
struct RPCDeathPlayerPoolView {
 DWORD prefix0;
 PLAYERID localId;
 std::string localName;
 DWORD field22;
 CLocalPlayer *localPlayer;
 BOOL slots[MAX_PLAYERS];
 BYTE middleGap[4016];
 CNetPlayer *players[MAX_PLAYERS];
 PLAYERID GetLocalPlayerID() { return localId; }
 PCHAR GetLocalPlayerName() { return (PCHAR)localName.c_str(); }
 CLocalPlayer *GetLocalPlayer() { return localPlayer; }
 BOOL GetSlotState(PLAYERID id) { if(id>=MAX_PLAYERS)return FALSE; return slots[id]; }
 CRemotePlayer *GetAt(PLAYERID id) { return ((CPlayerPool*)this)->GetAt(id); }
 PCHAR GetPlayerName(PLAYERID id);
};
#pragma pack(pop)
typedef char R5DeathLocalNameOffset[(offsetof(RPCDeathPlayerPoolView,localName)==6)?1:-1];
typedef char R5DeathSlotsOffset[(offsetof(RPCDeathPlayerPoolView,slots)==0x2A)?1:-1];
typedef char R5DeathPlayersOffset[(offsetof(RPCDeathPlayerPoolView,players)==0x1F8A)?1:-1];


#pragma pack(push,1)
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
#pragma pack(push,1)
struct R5ScoreboardRow
{
 DWORD color;
 char name[29];
 DWORD id;
 int score;
 DWORD ping;
};
typedef char VerifyR5ScoreboardRow45[(sizeof(R5ScoreboardRow)==45)?1:-1];
class R5ScoreboardUpdateView
{
public:
 BOOL enabled;
 BYTE prefix4[0x34-4];
 CDXUTDialog *dialog;
 CDXUTListBox *listbox;
 int field3C;
 int sortMode;
 void UpdatePlayers();
};
#pragma pack(pop)
void R5ScoreboardUpdateView::UpdatePlayers()
{
 if(!pNetGame || !pNetGame->GetPlayerPool() || !enabled || !dialog) return;
 int selected=listbox->GetSelectedIndex();
 CDXUTScrollBar *scroll=(CDXUTScrollBar*)((BYTE*)listbox+0x5D);
 int scrollPosition=scroll->GetTrackPos();
 listbox->RemoveAllItems();
 RPCDeathPlayerPoolView *pool=(RPCDeathPlayerPoolView*)pNetGame->GetPlayerPool();
 R5ScoreboardPlayerQueriesView *queries=(R5ScoreboardPlayerQueriesView*)pool;
 int playerCount=queries->GetCount(FALSE)+1;
 int localId=pool->localId;
 R5ScoreboardRow *rows=(R5ScoreboardRow*)calloc(playerCount,sizeof(R5ScoreboardRow));
 strcpy(rows[0].name,pool->GetLocalPlayerName());
 rows[0].color=pool->localPlayer->GetPlayerColorAsARGB();
 rows[0].score=pool->prefix0;
 rows[0].ping=pool->field22;
 rows[0].id=pool->localId;
 int row=1;
 for(int id=0;id<MAX_PLAYERS;id++)
 {
  if(pool->GetSlotState((PLAYERID)id)==TRUE && id!=localId && !queries->IsNPC((PLAYERID)id))
  {
   rows[row].ping=queries->GetPing((PLAYERID)id);
   strcpy(rows[row].name,pool->GetPlayerName((PLAYERID)id));
   rows[row].score=queries->GetScore((PLAYERID)id);
   rows[row].color=queries->GetRemote((PLAYERID)id)->GetPlayerColorAsARGB();
   rows[row].id=id;
   ++row;
  }
 }
 if(sortMode==1)
 {
  for(int i=0;i<playerCount-1;i++)
   for(int j=0;j<playerCount-1-i;j++)
    if(strcmp(rows[j+1].name,rows[j].name)<0)
    {
     R5ScoreboardRow temporary=rows[j]; rows[j]=rows[j+1]; rows[j+1]=temporary;
    }
 }
 else if(sortMode==2)
 {
  for(int i=0;i<playerCount-1;i++)
   for(int j=0;j<playerCount-1-i;j++)
    if(rows[j+1].score>rows[j].score)
    {
     R5ScoreboardRow temporary=rows[j]; rows[j]=rows[j+1]; rows[j+1]=temporary;
    }
 }
 // Original live scratch interval [ESP+0x60, ESP+0x164): 260 bytes.
 // Source declaration was not uniquely recoverable (257..260 round here).
 char buffer[260];
 for(int index=0;index<playerCount;index++)
 {
  sprintf(buffer,"%u",rows[index].id);
  ((R5ListBoxItemOwnerView*)listbox)->AddItem(buffer,(void*)rows[index].id,rows[index].color);
  DXUTListBoxItem *item=listbox->GetItem(index);
  if(item && rows[index].name[0])
  {
   ((R5ListBoxItemOwnerView*)listbox)->SetColumnText(index,0,rows[index].name);
   sprintf(buffer,"%d",rows[index].score);
   ((R5ListBoxItemOwnerView*)listbox)->SetColumnText(index,1,buffer);
   sprintf(buffer,"%u",rows[index].ping);
   ((R5ListBoxItemOwnerView*)listbox)->SetColumnText(index,2,buffer);
  }
 }
 if(selected>=0) listbox->SelectItem(selected);
 else listbox->SelectItem(-1);
 free(rows);
 scroll->SetTrackPos(scrollPosition);
}
