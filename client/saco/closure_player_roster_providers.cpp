#include "main.h"
#include <stddef.h>
extern CNetGame *pNetGame;
extern CGame *pGame;
extern CScoreBoard *pScoreBoard;
// Non-owning views of existing objects; never allocated or constructed.
#pragma pack(push,1)
class R5RosterRemotePlayerView {
public:
 int field0;
 int activeState;
 BYTE gap8[0x10A-8];
 BYTE playerState;
 BYTE field10B;
 int isNPC;
 BYTE gap110[0x1DD-0x110];
 CPlayerPed *ped;
 DWORD field1E1;
 PLAYERID playerId;
 BYTE tail[0x1FD-0x1E7];
 void Remove();
};
class R5RosterPlayerPoolView {
public:
 int localScore;
 PLAYERID localId;
 std::string localName;
 DWORD localPing;
 CLocalPlayer *localPlayer;
 BOOL slots[MAX_PLAYERS];
 BYTE middleGap[4016];
 CNetPlayer *players[MAX_PLAYERS];
 int largestId;
 CRemotePlayer *GetAt(PLAYERID id) { return ((CPlayerPool*)this)->GetAt(id); }
 BOOL GetSlotState(PLAYERID id) { if(id>=MAX_PLAYERS)return FALSE; return slots[id]; }
 void UpdateLargestId();
 BOOL New(PLAYERID id,PCHAR name,BOOL isNPC);
 void UpdateScore(PLAYERID id,int score) {
  if(id==localId) localScore=score;
  else { if(id>MAX_PLAYERS)return; CNetPlayer *p=players[id]; if(p)p->field_4=score; }
 }
 void UpdatePing(PLAYERID id,DWORD ping) {
  if(id==localId) localPing=ping;
  else { if(id>MAX_PLAYERS)return; CNetPlayer *p=players[id]; if(p)p->field_C=ping; }
 }
};
// Exact token-identical declaration of the already accepted UpdatePlayers owner.
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
typedef char R5RosterPlayers[(offsetof(R5RosterPlayerPoolView,players)==0x1F8A)?1:-1];
typedef char R5RosterSlots[(offsetof(R5RosterPlayerPoolView,slots)==0x2A)?1:-1];
typedef char R5RosterLargest[(offsetof(R5RosterPlayerPoolView,largestId)==0x2F3A)?1:-1];
typedef char R5RosterRemotePed[(offsetof(R5RosterRemotePlayerView,ped)==0x1DD)?1:-1];
typedef char R5RosterRemoteId[(offsetof(R5RosterRemotePlayerView,playerId)==0x1E5)?1:-1];
typedef char R5RosterRemoteSize[(sizeof(R5RosterRemotePlayerView)==sizeof(CRemotePlayer))?1:-1];
typedef char R5RosterNetPlayerSize[(sizeof(CNetPlayer)==0x30)?1:-1];

void R5RosterPlayerPoolView::UpdateLargestId()
{
 int largest=0;
 for(int id=0;id<MAX_PLAYERS;++id) if(slots[id]) largest=id;
 largestId=largest;
}
BOOL R5RosterPlayerPoolView::New(PLAYERID id,PCHAR name,BOOL isNPC)
{
 CNetPlayer *player=new CNetPlayer(name,isNPC);
 if(id>MAX_PLAYERS)return FALSE;
 players[id]=player;
 if(player) {
  ((R5RosterRemotePlayerView*)player->m_pRemotePlayer)->playerId=id;
  if(isNPC) ((R5RosterRemotePlayerView*)GetAt(id))->isNPC=TRUE;
  slots[id]=TRUE;
  UpdateLargestId();
  return TRUE;
 }
 return FALSE;
}
void R5RosterRemotePlayerView::Remove()
{
 if(ped) {
  ((CRemotePlayer*)this)->ResetAllSyncAttributes();
  pGame->DeletePlayerPed(ped);
  ped=NULL;
 }
 playerState=0;
 activeState=1;
}
