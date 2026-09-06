#include "main.h"
extern CGame *pGame;
extern BOOL bUsedPlayerSlots[PLAYER_PED_SLOTS];
typedef char R5RemotePlayerSize[(sizeof(CRemotePlayer)==0x1FD)?1:-1];
typedef char R5NetPlayerSize[(sizeof(CNetPlayer)==0x30)?1:-1];
// Real nonvirtual destructors and CGame ped ownership helper.
CNetPlayer::~CNetPlayer()
{
    SAFE_DELETE(m_pRemotePlayer);
}
CRemotePlayer::~CRemotePlayer()
{
    if (field_1F9) {
        pGame->DisableMarker(field_1F9);
        field_1F9=0;
    }
    field_1E9=0;
    if (m_pPlayerPed) {
        pGame->DeletePlayerPed(m_pPlayerPed);
        m_pPlayerPed=NULL;
    }
}
BOOL CGame::DeletePlayerPed(CPlayerPed *pPlayerPed)
{
    if (pPlayerPed) {
        BYTE playerNumber=pPlayerPed->m_bytePlayerNumber;
        delete pPlayerPed;
        bUsedPlayerSlots[playerNumber]=FALSE;
        return TRUE;
    }
    return FALSE;
}
void CGame::DisableMarker(DWORD dwMarkerID)
{
    ScriptCommand(&disable_marker,dwMarkerID);
}
