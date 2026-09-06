// 0.2.5 animation transfers and R5 camera RPC reconstructions.
#include "main.h"
extern CNetGame *pNetGame;
extern CGame *pGame;
struct RPCAnimationRemoteView {
 BYTE first[4]; BOOL showNameTag; BYTE gap8[0x1b9]; BOOL customAnimation;
 BYTE gap1c5[0x18]; CPlayerPed *ped;
 CPlayerPed *GetPlayerPed() { return ped; }
 void ShowNameTag(BYTE show) { showNameTag=(show!=0); }
};
struct RPCAnimationNetPlayerView { BYTE gap[0x10]; RPCAnimationRemoteView *remote; };
struct RPCAnimationPlayerPoolView {
 BYTE first[4]; WORD localId; BYTE gap6[0x20]; CLocalPlayer *local;
 BOOL slots[1004]; BYTE gapfda[4016]; RPCAnimationNetPlayerView *players[1004];
 WORD GetLocalPlayerID() { return localId; }
 CLocalPlayer *GetLocalPlayer() { return local; }
 BOOL GetSlotState(WORD id) { if(id>=1004)return FALSE; return slots[id]; }
 RPCAnimationRemoteView *GetAt(WORD id) { if(id>1004)return NULL; RPCAnimationNetPlayerView *p=players[id]; if(p)return p->remote; return NULL; }
};
struct RPCAnimationNetGameView { BYTE gap[0x3de]; NETGAME_POOLS *pools; };
inline RPCAnimationPlayerPoolView *RPCAnimationPlayers() { return (RPCAnimationPlayerPoolView*)((RPCAnimationNetGameView*)pNetGame)->pools->pPlayerPool; }
struct RPCCameraLocalView { BYTE gap[0x314]; BOOL cameraControlled; };
struct RPCCameraObjectPoolView {
 int lastId; BOOL slots[1000]; CEntity *objects[1000];
 CEntity *GetAt(WORD id) { if(id>1000)return NULL; if(slots[id])return objects[id]; return NULL; }
 BOOL GetSlotState(WORD id) { if(id>1000)return FALSE; return slots[id]; }
};
inline RPCCameraObjectPoolView *RPCCameraObjects() { return (RPCCameraObjectPoolView*)((RPCAnimationNetGameView*)pNetGame)->pools->pObjectPool; }
void ScrApplyAnimationTransfer(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;
	PlayerID sender = rpcParams->sender;
	RakNet::BitStream bsData(Data,(iBitLength/8)+1,false);

	PLAYERID playerId;
	BYTE byteAnimLibLen;
	BYTE byteAnimNameLen;
	char szAnimLib[256];
	char szAnimName[256];
	float fS;
	bool opt1,opt2,opt3,opt4;
	int  opt5;
	RPCAnimationPlayerPoolView *pPlayerPool=NULL;
	CPlayerPed *pPlayerPed=NULL;

	memset(szAnimLib,0,256);
	memset(szAnimName,0,256);

	bsData.Read(playerId);
	bsData.Read(byteAnimLibLen);
	bsData.Read(szAnimLib,byteAnimLibLen);
	bsData.Read(byteAnimNameLen);
	bsData.Read(szAnimName,byteAnimNameLen);
	bsData.Read(fS);
	bsData.Read(opt1);
	bsData.Read(opt2);
	bsData.Read(opt3);
	bsData.Read(opt4);
	bsData.Read(opt5);

	szAnimLib[byteAnimLibLen] = '\0';
	szAnimName[byteAnimNameLen] = '\0';

	pPlayerPool = RPCAnimationPlayers();

	if(pPlayerPool) {
		// Get the CPlayerPed for this player
		if(playerId == pPlayerPool->GetLocalPlayerID()) {
			pPlayerPed = pPlayerPool->GetLocalPlayer()->GetPlayerPed();
		}
		else {
			if(pPlayerPool->GetSlotState(playerId)) {
				RPCAnimationRemoteView *remote=pPlayerPool->GetAt(playerId);
				if(remote) { pPlayerPed=remote->GetPlayerPed(); remote->customAnimation=1; }
			}
		}
		if(pPlayerPed) {
			
				pPlayerPed->ApplyAnimation(szAnimName,szAnimLib,fS,
					(int)opt1,(int)opt2,(int)opt3,(int)opt4,(int)opt5);
			
		}
	}
}

void ScrClearAnimationsTransfer(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;
	RakNet::BitStream bsData(Data,(iBitLength/8)+1,false);

	PLAYERID playerId;
	bsData.Read(playerId);
	MATRIX4X4 mat;

	RPCAnimationPlayerPoolView *pPlayerPool=NULL;
	CPlayerPed *pPlayerPed=NULL;

	pPlayerPool = RPCAnimationPlayers();

	if(pPlayerPool) {
		// Get the CPlayerPed for this player
		if(playerId == pPlayerPool->GetLocalPlayerID()) {
			pPlayerPed = pPlayerPool->GetLocalPlayer()->GetPlayerPed();
		}
		else {
			if(pPlayerPool->GetSlotState(playerId)) {
				RPCAnimationRemoteView *remote=pPlayerPool->GetAt(playerId);
				if(remote) { pPlayerPed=remote->GetPlayerPed(); remote->customAnimation=0; }
			}
		}
		if(pPlayerPed) {
			
				
				pPlayerPed->GetMatrix(&mat);
				pPlayerPed->TeleportTo(mat.pos.X,mat.pos.Y,mat.pos.Z);

			
		}
	}
}

void ScrSetFightingStyleTransfer(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;
	RakNet::BitStream bsData(Data,(iBitLength/8)+1,false);

	PLAYERID playerId;
	BYTE byteFightingStyle = 0;

	bsData.Read(playerId);
	bsData.Read(byteFightingStyle);
	
	RPCAnimationPlayerPoolView *pPlayerPool = RPCAnimationPlayers();
	CPlayerPed *pPlayerPed = 0;

	if(pPlayerPool) {
		if(playerId == pPlayerPool->GetLocalPlayerID()) {
			pPlayerPed = pPlayerPool->GetLocalPlayer()->GetPlayerPed();
		}
		else {
			if(pPlayerPool->GetSlotState(playerId)) {
				pPlayerPed = pPlayerPool->GetAt(playerId)->GetPlayerPed();
			}
		}
		if(pPlayerPed) {
			
				
				pPlayerPed->SetFightingStyle(byteFightingStyle);

			
		}
	}
}

void ScrShowNameTagTransfer(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;
	PlayerID sender = rpcParams->sender;

	RakNet::BitStream bsData(Data,(iBitLength/8)+1,false);
	PLAYERID playerId;
	BYTE byteShow;

	bsData.Read(playerId);
	bsData.Read(byteShow);

	if (RPCAnimationPlayers()->GetSlotState(playerId))
	{
		RPCAnimationPlayers()->GetAt(playerId)->ShowNameTag(byteShow);
	}
}

// R5-specific camera attachment; object is viewed only as CEntity at offset 0.
void ScrAttachCameraToObjectTransfer(RPCParameters *rpcParams) {
 PCHAR Data=reinterpret_cast<PCHAR>(rpcParams->input);
 int iBitLength=rpcParams->numberOfBitsOfData;
 WORD objectId;
 RakNet::BitStream bsData(Data,(iBitLength/8)+1,false);
 bsData.Read(objectId);
 if(RPCCameraObjects()->GetSlotState(objectId)) {
  ((RPCCameraLocalView*)pNetGame->GetPlayerPool()->GetLocalPlayer())->cameraControlled=TRUE;
  pGame->GetCamera()->FUNC_1009D660(RPCCameraObjects()->GetAt(objectId));
 }
}
void ScrInterpolateCameraTransfer(RPCParameters *rpcParams) {
 PCHAR Data=reinterpret_cast<PCHAR>(rpcParams->input);
 int iBitLength=rpcParams->numberOfBitsOfData;
 RakNet::BitStream bsData(Data,(iBitLength/8)+1,false);
 bool position=true;
 VECTOR from,to;
 int time;
 BYTE cut;
 bsData.Read(position);
 bsData.Read(from.X); bsData.Read(from.Y); bsData.Read(from.Z);
 bsData.Read(to.X); bsData.Read(to.Y); bsData.Read(to.Z);
 bsData.Read(time); bsData.Read(cut);
 if(cut<1 || cut>2) cut=2;
 if(time>0) {
  ((RPCCameraLocalView*)pNetGame->GetPlayerPool()->GetLocalPlayer())->cameraControlled=TRUE;
  if(position) pGame->GetCamera()->InterpolatePosition(&from,&to,time,cut);
  else pGame->GetCamera()->InterpolateLookAt(&from,&to,time,cut);
 }
}
