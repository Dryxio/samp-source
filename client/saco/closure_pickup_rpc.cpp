// 0.2.5 RPC transfer with disassembly-verified R5 pool access.
#include "main.h"
extern CNetGame *pNetGame;
struct R5PickupRecordMore { int iModel; int iType; float fX; float fY; float fZ; };
class CPickupPoolCreate {
 int m_iPickupCount;
 DWORD m_dwHnd[MAX_PICKUPS];
 int m_iNetworkIds[MAX_PICKUPS];
 DWORD m_iTimer[MAX_PICKUPS];
 R5_DROPPED_WEAPON m_droppedWeapon[MAX_PICKUPS];
 R5PickupRecordMore m_Pickups[MAX_PICKUPS];
public:
 void New(R5PickupRecordMore *pPickup,int iPickup);
};
struct PickupRPCNetGameView { BYTE unknown[0x3de]; NETGAME_POOLS *pools; };
static inline CPickupPool *RPCPickupPool()
{
 return ((PickupRPCNetGameView*)pNetGame)->pools->pPickupPool;
}
void PickupRPCTransfer(RPCParameters *rpcParams)
{
 PCHAR Data=reinterpret_cast<PCHAR>(rpcParams->input);
 int iBitLength=rpcParams->numberOfBitsOfData;
 RakNet::BitStream bsData(Data,(iBitLength/8)+1,false);
 R5PickupRecordMore Pickup;
 int iIndex;
 bsData.Read(iIndex);
 bsData.Read((PCHAR)&Pickup,sizeof(R5PickupRecordMore));
 CPickupPoolCreate *pPickupPool=(CPickupPoolCreate*)RPCPickupPool();
 if(pPickupPool)pPickupPool->New(&Pickup,iIndex);
}
void DestroyPickupRPCTransfer(RPCParameters *rpcParams)
{
 PCHAR Data=reinterpret_cast<PCHAR>(rpcParams->input);
 int iBitLength=rpcParams->numberOfBitsOfData;
 RakNet::BitStream bsData(Data,(iBitLength/8)+1,false);
 int iIndex;
 bsData.Read(iIndex);
 CPickupPool *pPickupPool=RPCPickupPool();
 if(pPickupPool)pPickupPool->Destroy(iIndex);
}
void DestroyWeaponPickupRPCTransfer(RPCParameters *rpcParams)
{
 PCHAR Data=reinterpret_cast<PCHAR>(rpcParams->input);
 int iBitLength=rpcParams->numberOfBitsOfData;
 RakNet::BitStream bsData(Data,(iBitLength/8)+1,false);
 BYTE byteIndex;
 bsData.Read(byteIndex);
 CPickupPool *pPickupPool=RPCPickupPool();
 pPickupPool->DestroyDropped(byteIndex);
}
