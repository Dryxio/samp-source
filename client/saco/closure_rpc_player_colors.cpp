#include "main.h"
extern CNetGame *pNetGame;

void CLocalPlayer::SetPlayerColor(DWORD dwColor)
{
 SetRadarColor(pNetGame->GetPlayerPool()->GetLocalPlayerID(),dwColor);
}
void CRemotePlayer::SetPlayerColor(DWORD dwColor)
{
 SetRadarColor(m_PlayerID,dwColor);
}

void ScrSetPlayerColorTransfer(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;
	PlayerID sender = rpcParams->sender;

	RakNet::BitStream bsData(Data,(iBitLength/8)+1,false);
	CPlayerPool *pPlayerPool = pNetGame->GetPlayerPool();
	PLAYERID playerId;
	DWORD dwColor;

	bsData.Read(playerId);
	bsData.Read(dwColor);

	if(playerId == pPlayerPool->GetLocalPlayerID()) {
		pPlayerPool->GetLocalPlayer()->SetPlayerColor(dwColor);
	} else {
		CRemotePlayer *pPlayer = pPlayerPool->GetAt(playerId);
		if(pPlayer)	pPlayer->SetPlayerColor(dwColor);
	}
}

