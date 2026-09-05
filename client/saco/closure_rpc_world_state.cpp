// 0.2.5 state RPC transfers; verified partial R5 views only.
#include "main.h"
extern CNetGame *pNetGame;
extern CGame *pGame;
extern RakNetStatisticsStruct RakServerStats;
struct RPCStateNetGameView { BYTE unknown[0x3d5]; struc_41 *settings; };
struct RPCCheckpointGameView { BYTE unknown[0x29]; BOOL race; BYTE gap[0x20]; BOOL normal; };
struct RPCClassSelectionView { BYTE unknown[0x2fa]; BOOL wantsAnotherClass; };


void WorldTimeTransfer(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;
	//PlayerID sender = rpcParams->sender;

	RakNet::BitStream bsData(Data,(iBitLength/8)+1,false);
	BYTE byteWorldTime;
	bsData.Read(byteWorldTime);
	((RPCStateNetGameView*)pNetGame)->settings->field_2C = byteWorldTime;	
}

void ToggleClockTransfer(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;
	//PlayerID sender = rpcParams->sender;

	RakNet::BitStream bsData(Data,(iBitLength/8)+1,false);
	BYTE byteClock;
	bsData.Read(byteClock);
	pGame->EnableClock(byteClock);	
	if (byteClock)
	{
		((RPCStateNetGameView*)pNetGame)->settings->field_1F = 0;
	}
	else
	{
		((RPCStateNetGameView*)pNetGame)->settings->field_1F = 1;
		pGame->GetWorldTime((BYTE*)&((RPCStateNetGameView*)pNetGame)->settings->field_2C, (BYTE*)&((RPCStateNetGameView*)pNetGame)->settings->field_2D);
	}
}

void SvrStatsTransfer(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;
	RakNet::BitStream bsData(Data,(iBitLength/8)+1,false);
	bsData.Read((char *)&RakServerStats,sizeof(RakNetStatisticsStruct));
}

void ScrEnableStuntBonusTransfer(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;
	RakNet::BitStream bsData(Data,(iBitLength/8)+1,false);

	bool bStuntBonusEnabled;
	bsData.Read(bStuntBonusEnabled);
	pGame->EnableStuntBonus(bStuntBonusEnabled);
}

void ScrResetMoneyTransfer(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;
	PlayerID sender = rpcParams->sender;

	pGame->ResetLocalMoney();
}

void DisableCheckpointTransfer(RPCParameters *rpcParams) { ((RPCCheckpointGameView*)pGame)->normal=FALSE; }

void DisableRaceCheckpointTransfer(RPCParameters *rpcParams) { ((RPCCheckpointGameView*)pGame)->race=FALSE; }

void ScrForceSpawnSelectionTransfer(RPCParameters *rpcParams) { ((RPCClassSelectionView*)pNetGame->GetPlayerPool()->GetLocalPlayer())->wantsAnotherClass=TRUE; }
