// 0.2.5 RPC transfers with explicitly verified partial R5 views.
#include "main.h"
extern CNetGame *pNetGame;
extern CGame *pGame;
struct RPCSoundGameView { CAudio *audio; CAudio *GetAudio() { return audio; } };
struct RPCBoundsSettingsView { BYTE gap[5]; float bounds[4]; };
struct RPCBoundsNetGameView { BYTE gap[0x3d5]; RPCBoundsSettingsView *settings; };
struct RPCObjectStatePoolView {
 int lastId; BOOL slots[1000]; CObject *objects[1000];
 CObject *GetAt(WORD id) { if(id>1000)return NULL; if(slots[id])return objects[id]; return NULL; }
};
struct RPCObjectStateNetGameView { BYTE gap[0x3de]; NETGAME_POOLS *pools; };
struct RPCTeamLocalView { BYTE gap[0x10c]; BYTE team; };
struct RPCTeamRemoteView { BYTE gap[0x109]; BYTE team; };
struct RPCObjectCameraView { BYTE gap[0x53]; BYTE noCameraCollision; };
void ScrPlaySoundTransfer(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;
	PlayerID sender = rpcParams->sender;

	RakNet::BitStream bsData(Data,(iBitLength/8)+1,false);

	int iSound;
	float fX, fY, fZ;
	bsData.Read(iSound);
	bsData.Read(fX);
	bsData.Read(fY);
	bsData.Read(fZ);

	((RPCSoundGameView*)pGame)->GetAudio()->PlaySound(iSound, fX, fY, fZ);
}

void ScrSetWorldBoundsTransfer(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;
	PlayerID sender = rpcParams->sender;

	RakNet::BitStream bsData(Data,(iBitLength/8)+1,false);
	bsData.Read(((RPCBoundsNetGameView*)pNetGame)->settings->bounds[0]);
	bsData.Read(((RPCBoundsNetGameView*)pNetGame)->settings->bounds[1]);
	bsData.Read(((RPCBoundsNetGameView*)pNetGame)->settings->bounds[2]);
	bsData.Read(((RPCBoundsNetGameView*)pNetGame)->settings->bounds[3]);
}

void ScrSetPlayerTeamTransfer(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;
	PlayerID sender = rpcParams->sender;

	PLAYERID playerId;
	BYTE byteTeam;

	RakNet::BitStream bsData(Data,(iBitLength/8)+1,false);

	CPlayerPool *pPlayerPool = pNetGame->GetPlayerPool();

	bsData.Read(playerId);
	bsData.Read(byteTeam);
	
	if (playerId == pPlayerPool->GetLocalPlayerID()) {
		((RPCTeamLocalView*)pPlayerPool->GetLocalPlayer())->team = byteTeam;
	} else {
		CRemotePlayer *pPlayer = pPlayerPool->GetAt(playerId);
		if(pPlayer) ((RPCTeamRemoteView*)pPlayer)->team = byteTeam;
	}
}

void ScrSetObjectPosTransfer(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;
	PlayerID sender = rpcParams->sender;

	WORD byteObjectID;
	float fX, fY, fZ, fRotation;
	RakNet::BitStream bsData(Data,(iBitLength/8)+1,false);
	bsData.Read(byteObjectID);
	bsData.Read(fX);
	bsData.Read(fY);
	bsData.Read(fZ);
	bsData.Read(fRotation);

	RPCObjectStatePoolView* pObjectPool = (RPCObjectStatePoolView*)((RPCObjectStateNetGameView*)pNetGame)->pools->pObjectPool;
	CObject*		pObject		=	pObjectPool->GetAt(byteObjectID);
	if (pObject)
	{
		((CEntity*)pObject)->TeleportTo(fX, fY, fZ);
	}
}

void ScrRespawnVehicleTransfer(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;
	PlayerID sender = rpcParams->sender;

	VEHICLEID VehicleID;
	RakNet::BitStream bsData(Data,(iBitLength/8)+1,false);
	bsData.Read(VehicleID);	
}

// R5 RPC169: reconstructed field update; no original 0.2.5 handler claimed.
void ScrSetObjectNoCameraCollisionTransfer(RPCParameters *rpcParams) {
 PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
 int iBitLength = rpcParams->numberOfBitsOfData;
 WORD objectId;
 RakNet::BitStream bsData(Data,(iBitLength/8)+1,false);
 bsData.Read(objectId);
 RPCObjectStatePoolView *pool=(RPCObjectStatePoolView*)((RPCObjectStateNetGameView*)pNetGame)->pools->pObjectPool;
 CObject *object=pool->GetAt(objectId);
 if(object) ((RPCObjectCameraView*)object)->noCameraCollision=1;
}
