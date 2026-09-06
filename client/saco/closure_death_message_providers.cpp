#include "main.h"
#include <stddef.h>
// Non-owning layout view of existing CPlayerPool storage. Never constructed/allocated.
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

extern CNetGame *pNetGame;
DWORD CLocalPlayer::GetPlayerColorAsARGB()
{
 return (TranslateColorCodeToRGBA(pNetGame->GetPlayerPool()->GetLocalPlayerID()) >> 8) | 0xFF000000;
}
DWORD CRemotePlayer::GetPlayerColorAsARGB()
{
 return (TranslateColorCodeToRGBA(m_PlayerID) >> 8) | 0xFF000000;
}
PCHAR RPCDeathPlayerPoolView::GetPlayerName(PLAYERID id)
{
 if(id==localId)return (PCHAR)localName.c_str();
 if(id>MAX_PLAYERS)return NULL;
 CNetPlayer *player=players[id];
 if(!player)return NULL;
 return (PCHAR)player->m_PlayerName.c_str();
}
