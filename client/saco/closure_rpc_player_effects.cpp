// 0.2.5 transfers plus verified R5 local-player reset dependency.

#include "main.h"

extern CNetGame *pNetGame;

extern CGame *pGame;

extern CChatWindow *pChatWindow;

const SCRIPT_COMMAND toggle_widescreen={0x02A3,"i"};

const SCRIPT_COMMAND create_explosion_with_radius={0x0948,"fffii"};

class LocalPlayerRPCResetView {
 BYTE unknown[0x2da];
public:
 DWORD field_2DA,field_2DE,field_2E2;
 void SetStatePair(int first,int second);
};

void ScrSetPlayerPosTransfer(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;
	PlayerID sender = rpcParams->sender;

	RakNet::BitStream bsData(Data,(iBitLength/8)+1,false);

	CPlayerPool *pPlayerPool=pNetGame->GetPlayerPool();
 if(!pPlayerPool)return;
 CLocalPlayer *pLocalPlayer=pPlayerPool->GetLocalPlayer();
 if(!pLocalPlayer)return;

	VECTOR vecPos;

	bsData.Read(vecPos.X);
	bsData.Read(vecPos.Y);
	bsData.Read(vecPos.Z);

	((LocalPlayerRPCResetView*)pLocalPlayer)->field_2E2=0;
 ((LocalPlayerRPCResetView*)pLocalPlayer)->SetStatePair(0,0);
 pLocalPlayer->GetPlayerPed()->TeleportTo(vecPos.X,vecPos.Y,vecPos.Z);
}

void ScrSetPlayerPosFindZTransfer(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;
	PlayerID sender = rpcParams->sender;

	RakNet::BitStream bsData(Data,(iBitLength/8)+1,false);

	CLocalPlayer *pLocalPlayer = pNetGame->GetPlayerPool()->GetLocalPlayer();

	VECTOR vecPos;

	bsData.Read(vecPos.X);
	bsData.Read(vecPos.Y);
	bsData.Read(vecPos.Z);

	vecPos.Z = pGame->FindGroundZForCoord(vecPos.X, vecPos.Y, vecPos.Z);
	vecPos.Z += 1.5f;

	((LocalPlayerRPCResetView*)pLocalPlayer)->field_2E2=0;
 ((LocalPlayerRPCResetView*)pLocalPlayer)->SetStatePair(0,0);
 pLocalPlayer->GetPlayerPed()->TeleportTo(vecPos.X, vecPos.Y, vecPos.Z);
}

void ScrResetPlayerWeaponsTransfer(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;
	PlayerID sender = rpcParams->sender;

	CPlayerPed *pPlayerPed = pNetGame->GetPlayerPool()->GetLocalPlayer()->GetPlayerPed();
	pPlayerPed->ClearAllWeapons();
	//pChatWindow->AddDebugMessage("Cleared weapons");
}

void ScrSetCameraBehindPlayerTransfer(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;
	PlayerID sender = rpcParams->sender;

	pGame->GetCamera()->SetBehindPlayer();	
}

void ScrCreateExplosionTransfer(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;
	PlayerID sender = rpcParams->sender;

	RakNet::BitStream bsData(Data,(iBitLength/8)+1,false);
	float X, Y, Z, Radius;
	int   iType;

	bsData.Read(X);
	bsData.Read(Y);
	bsData.Read(Z);
	bsData.Read(iType);
	bsData.Read(Radius);

	ScriptCommand(&create_explosion_with_radius, X, Y, Z, iType, Radius);
}

void ScrToggleWidescreenTransfer(RPCParameters* rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;
	RakNet::BitStream bsData(Data,(iBitLength/8)+1,false);

	BYTE bToggle;
	bsData.Read(bToggle);

	pChatWindow->AddDebugMessage("Widescreen = %i", bToggle);

	ScriptCommand(&toggle_widescreen, bToggle);
}

void LocalPlayerRPCResetView::SetStatePair(int first,int second)
{
 field_2DA=first;
 field_2DE=second;
}

