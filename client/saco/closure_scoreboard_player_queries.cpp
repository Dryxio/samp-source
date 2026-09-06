// Real independent player queries required by scoreboard population.
#include "main.h"
#include <stddef.h>
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
int R5ScoreboardPlayerQueriesView::GetCount(BOOL includeNPC)
{
 int count=0;
 if(includeNPC)
 {
  for(int i=0;i<MAX_PLAYERS;i++) if(GetSlotState((PLAYERID)i)==TRUE) ++count;
 }
 else
 {
  for(int i=0;i<MAX_PLAYERS;i++) if(GetSlotState((PLAYERID)i)==TRUE && !players[(PLAYERID)i]->m_bIsNPC) ++count;
 }
 return count;
}
BOOL R5ScoreboardPlayerQueriesView::IsNPC(PLAYERID id)
{
 if(id>MAX_PLAYERS) return FALSE;
 CNetPlayer *player=players[id];
 if(!player) return FALSE;
 return player->m_bIsNPC;
}
int R5ScoreboardPlayerQueriesView::GetScore(PLAYERID id)
{
 if(id>MAX_PLAYERS) return 0;
 CNetPlayer *player=players[id];
 if(!player) return 0;
 return player->field_4;
}
int R5ScoreboardPlayerQueriesView::GetPing(PLAYERID id)
{
 if(id>MAX_PLAYERS) return 0;
 CNetPlayer *player=players[id];
 if(!player) return 0;
 return player->field_C;
}
CRemotePlayer *R5ScoreboardPlayerQueriesView::GetRemote(PLAYERID id)
{
 if(id>MAX_PLAYERS) return NULL;
 CNetPlayer *player=players[id];
 if(player) return player->m_pRemotePlayer;
 return NULL;
}
