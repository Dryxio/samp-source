// R5 spectator/vehicle closures; 0.2.5 RPC and spectator logic adapted.
#include "main.h"
#include "game/util.h"
extern CNetGame *pNetGame;
struct RPCSpectateNetGameView { BYTE gap[0x3de]; NETGAME_POOLS *pools; };
struct RPCSpectateRemoteView { BYTE gap[0x10a]; BYTE state; BYTE GetState() { return state; } };
struct RPCSpectateNetPlayerView { BYTE gap[0x10]; RPCSpectateRemoteView *remote; };
struct RPCSpectatePlayerPoolView {
 BYTE gap[0x2a]; BOOL slots[1004]; BYTE gapfda[4016]; RPCSpectateNetPlayerView *players[1004];
 BOOL GetSlotState(WORD id) { if(id>=1004)return FALSE; return slots[id]; }
 RPCSpectateRemoteView *GetAt(WORD id) { if(id>1004)return NULL; RPCSpectateNetPlayerView *p=players[id]; if(p)return p->remote; return NULL; }
};
struct RPCSpectateVehiclePoolView {
 BYTE gap[0x1134]; CVehicle *vehicles[2000]; BOOL slots[2000];
 BOOL GetSlotState(WORD id) { if(id>=2000)return FALSE; return slots[id]; }
 CVehicle *GetAt(WORD id) { if(id>=2000)return NULL; if(slots[id])return vehicles[id]; return NULL; }
};
static inline RPCSpectateVehiclePoolView *RPCSpectateVehicles() { return (RPCSpectateVehiclePoolView*)((RPCSpectateNetGameView*)pNetGame)->pools->pVehiclePool; }
struct RPCSpectateLocalView {
 BYTE gap[0x30e]; BYTE m_byteSpectateMode; BYTE m_byteSpectateType;
 int m_SpectateID; BOOL m_bSpectateProcessed;
 void SpectatePlayer(WORD id); void SpectateVehicle(WORD id);
};
void RPCSpectateLocalView::SpectateVehicle(WORD id) {
 RPCSpectateVehiclePoolView *pool=RPCSpectateVehicles();
 if(pool && pool->GetSlotState(id)) { m_byteSpectateType=2; m_SpectateID=id; m_bSpectateProcessed=FALSE; }
}
void RPCSpectateLocalView::SpectatePlayer(WORD id) {
 RPCSpectatePlayerPoolView *pool=(RPCSpectatePlayerPoolView*)((RPCSpectateNetGameView*)pNetGame)->pools->pPlayerPool;
 if(pool && pool->GetSlotState(id)) {
  if(pool->GetAt(id)->GetState()!=0 && pool->GetAt(id)->GetState()!=32) {
   m_byteSpectateType=1; m_SpectateID=id; m_bSpectateProcessed=FALSE;
  }
 }
}
struct RPCVehicleAngleView {
 BYTE gap[0x44]; DWORD gtaId;
 void SetZAngle(float angle);
};
extern "C" const SCRIPT_COMMAND rpc_set_car_z_angle={0x0175,"if"};
void RPCVehicleAngleView::SetZAngle(float angle) {
 if(GamePool_Vehicle_GetAt(gtaId)) ScriptCommand(&rpc_set_car_z_angle,gtaId,angle);
}
#define SPECTATE_TYPE_FIXED 2
#define SPECTATE_TYPE_SIDE 3
void ScrPlayerSpectatePlayerTransfer(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;
	PlayerID sender = rpcParams->sender;

	RakNet::BitStream bsData(Data,(iBitLength/8)+1,false);
	
	PLAYERID playerId;
    BYTE byteMode;
	
	bsData.Read(playerId);
	bsData.Read(byteMode);

	switch (byteMode) {
		case SPECTATE_TYPE_FIXED:
			byteMode = 15;
			break;
		case SPECTATE_TYPE_SIDE:
			byteMode = 14;
			break;
		default:
			byteMode = 4;
	}
	RPCSpectateLocalView *pLocalPlayer = (RPCSpectateLocalView*)pNetGame->GetPlayerPool()->GetLocalPlayer();
	pLocalPlayer->m_byteSpectateMode = byteMode;
	pLocalPlayer->SpectatePlayer(playerId);
}

void ScrPlayerSpectateVehicleTransfer(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;
	PlayerID sender = rpcParams->sender;

	RakNet::BitStream bsData(Data,(iBitLength/8)+1,false);
	
	VEHICLEID VehicleID;
    BYTE byteMode;
	
	bsData.Read(VehicleID);
	bsData.Read(byteMode);

	//pChatWindow->AddDebugMessage("ScrPlayerSpectateVehicle(%u,%u)",VehicleID,byteMode);
    
	switch (byteMode) {
		case SPECTATE_TYPE_FIXED:
			byteMode = 15;
			break;
		case SPECTATE_TYPE_SIDE:
			byteMode = 14;
			break;
		default:
			byteMode = 3;
	}
	RPCSpectateLocalView *pLocalPlayer = (RPCSpectateLocalView*)pNetGame->GetPlayerPool()->GetLocalPlayer();
	pLocalPlayer->m_byteSpectateMode = byteMode;
	pLocalPlayer->SpectateVehicle(VehicleID);
}

void ScrRemovePlayerFromVehicleTransfer(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;
	PlayerID sender = rpcParams->sender;

	RakNet::BitStream bsData(Data,(iBitLength/8)+1,false);

	CPlayerPool *pPlayerPool = pNetGame->GetPlayerPool();
	pPlayerPool->GetLocalPlayer()->GetPlayerPed()->ExitCurrentVehicle();
}

void ScrSetVehicleZAngleTransfer(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;
	PlayerID sender = rpcParams->sender;

	RakNet::BitStream bsData(Data,(iBitLength/8)+1,false);
	VEHICLEID VehicleId;
	float fZAngle;
	bsData.Read(VehicleId);
	bsData.Read(fZAngle);

	RPCSpectateVehiclePoolView *pool=RPCSpectateVehicles();
 if(pool && pool->GetSlotState(VehicleId)) ((RPCVehicleAngleView*)pool->GetAt(VehicleId))->SetZAngle(fZAngle);
}

struct RPCExitVehicleIndexView {
 BYTE gap[0x2a4]; PED_TYPE *ped;
 int GetCurrentVehicleID() {
  if(!ped)return 0;
  VEHICLE_TYPE *vehicle=(VEHICLE_TYPE*)ped->pVehicle;
  return GamePool_Vehicle_GetIndex(vehicle);
 }
};
void CPlayerPed::ExitCurrentVehicle()
{
	if(!m_pPed) return;
	if(!GamePool_Ped_GetAt(m_dwGTAId)) return;
	if(IN_VEHICLE(m_pPed)) {
		if(((RPCExitVehicleIndexView*)this)->GetCurrentVehicleID()) {
			int iVehicleID;
			VEHICLE_TYPE *pVehicle = GamePool_Vehicle_GetAt(((RPCExitVehicleIndexView*)this)->GetCurrentVehicleID());
			if(pVehicle) {
				if( pVehicle->entity.nModelIndex != TRAIN_PASSENGER &&
					pVehicle->entity.nModelIndex != TRAIN_PASSENGER_LOCO ) {
					ScriptCommand(&make_actor_leave_car,m_dwGTAId,GetCurrentVehicleID());
				}
			}
		}
	}
}
