// 0.2.5 RPC/pool transfers. R5 partial views only; no full pool allocation claim.
#include "main.h"
extern CNetGame *pNetGame;
class RPCVehiclePoolView {
 BYTE unknown[0x1134];
 CVehicle *m_pVehicles[MAX_VEHICLES];
 BOOL m_bVehicleSlotState[MAX_VEHICLES];
public:
 CVehicle *GetAt(WORD id) { if(id>=MAX_VEHICLES || !m_bVehicleSlotState[id])return NULL; return m_pVehicles[id]; }
 BOOL GetSlotState(WORD id) { if(id>=MAX_VEHICLES)return FALSE; return m_bVehicleSlotState[id]; }
 void LinkToInterior(WORD id,int interior);
};
struct RPCPoolsNetGameView { BYTE unknown[0x3de]; NETGAME_POOLS *pools; };
static inline RPCVehiclePoolView *RPCVehiclePool() { return (RPCVehiclePoolView*)((RPCPoolsNetGameView*)pNetGame)->pools->pVehiclePool; }
static inline CMenuPool *RPCMenuPool() { return ((RPCPoolsNetGameView*)pNetGame)->pools->pMenuPool; }


void ScrSetVehiclePosTransfer(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;
	PlayerID sender = rpcParams->sender;

	RakNet::BitStream bsData(Data,(iBitLength/8)+1,false);
	WORD VehicleId;
	float fX, fY, fZ;
	bsData.Read(VehicleId);
	bsData.Read(fX);
	bsData.Read(fY);
	bsData.Read(fZ);

	if(pNetGame && RPCVehiclePool()) {
		if(RPCVehiclePool()->GetSlotState(VehicleId)) {
			RPCVehiclePool()->GetAt(VehicleId)->TeleportTo(fX, fY, fZ);
		}
	}
}

void ScrLinkVehicleTransfer(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;
	PlayerID sender = rpcParams->sender;

	RakNet::BitStream bsData(Data,(iBitLength/8)+1,false);

	WORD VehicleID;
	BYTE byteInterior;

	bsData.Read(VehicleID);
	bsData.Read(byteInterior);

	RPCVehiclePoolView *pVehiclePool = RPCVehiclePool();
	pVehiclePool->LinkToInterior(VehicleID, (int)byteInterior);
}

void ScrSetVehicleHealthTransfer(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;
	PlayerID sender = rpcParams->sender;

	float fVehicleHealth;
	WORD VehicleID;

	RakNet::BitStream bsData(Data,(iBitLength/8)+1,false);
	bsData.Read(VehicleID);
	bsData.Read(fVehicleHealth);

	if (RPCVehiclePool() && RPCVehiclePool()->GetSlotState(VehicleID))
	{
		CVehicle *pVehicle=RPCVehiclePool()->GetAt(VehicleID);
 if(pVehicle)pVehicle->SetHealth(fVehicleHealth);
	}
}

void ScrShowMenuTransfer(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;
	PlayerID sender = rpcParams->sender;

	RakNet::BitStream bsData(Data,(iBitLength/8)+1,false);
	
	if(!pNetGame) return;
	CMenuPool* pMenuPool = RPCMenuPool();

	BYTE byteMenuID;
	bsData.Read(byteMenuID);
	RPCMenuPool()->ShowMenu(byteMenuID);
}

void ScrHideMenuTransfer(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;
	PlayerID sender = rpcParams->sender;

	RakNet::BitStream bsData(Data,(iBitLength/8)+1,false);
	
	if(!pNetGame) return;
	CMenuPool* pMenuPool = RPCMenuPool();

	BYTE byteMenuID;
	bsData.Read(byteMenuID);
	RPCMenuPool()->HideMenu(byteMenuID);
}

void RPCVehiclePoolView::LinkToInterior(WORD id,int interior)
{
 if(m_bVehicleSlotState[id])m_pVehicles[id]->LinkToInterior(interior);
}

