// 0.2.5 transfer proposal; not accepted. Only the integrator compiles.
#include "main.h"
extern CGame *pGame;
extern CNetGame *pNetGame;
const SCRIPT_COMMAND is_pickup_picked_up = {0x0214, "i"};
struct R5PickupRecord { int iModel; int iType; float fX; float fY; float fZ; };
class CPickupPoolMore {
 int m_iPickupCount;
 DWORD m_dwHnd[MAX_PICKUPS];
 int m_iNetworkIds[MAX_PICKUPS];
 DWORD m_iTimer[MAX_PICKUPS];
 R5_DROPPED_WEAPON m_droppedWeapon[MAX_PICKUPS];
 R5PickupRecord m_Pickups[MAX_PICKUPS];
public:
 int FindNetworkId(int networkId);
 void PickedUp(int networkId);
 void Process();
 void New(int iModel,float fX,float fY,float fZ,DWORD dwAmmo,WORD fromDeadPlayer);
};
typedef char pickup_record_size[(sizeof(R5PickupRecord)==20)?1:-1];
typedef char pickup_pool_more_size[(sizeof(CPickupPoolMore)==0x23004)?1:-1];
int CPickupPoolMore::FindNetworkId(int networkId)
{
 for(int i=0;i!=MAX_PICKUPS;i++) if(m_iNetworkIds[i]==networkId) return i;
 return -1;
}
void CPickupPoolMore::PickedUp(int networkId)
{
 int iPickup=FindNetworkId(networkId);
 if(iPickup == -1) return;
 if(m_dwHnd[iPickup] != NULL && m_iTimer[iPickup] == 0) {
  if(m_droppedWeapon[iPickup].bDroppedWeapon) return;
  RakNet::BitStream bsPickup;
  bsPickup.Write(iPickup);
  pNetGame->GetRakClient()->RPC(RPC_PickedUpPickup,&bsPickup,HIGH_PRIORITY,RELIABLE_ORDERED,0,FALSE);
  m_iTimer[iPickup]=15;
 }
}
void CPickupPoolMore::Process()
{
 for(int i=0;i<MAX_PICKUPS;i++) {
  if(m_dwHnd[i]!=NULL) {
   if(m_droppedWeapon[i].bDroppedWeapon || m_Pickups[i].iType==14) {
    if(ScriptCommand(&is_pickup_picked_up,m_dwHnd[i])) {
     RakNet::BitStream bsPickup;
     if(m_droppedWeapon[i].bDroppedWeapon) {
      bsPickup.Write(m_droppedWeapon[i].fromPlayer);
      pNetGame->GetRakClient()->RPC(RPC_PickedUpWeapon,&bsPickup,HIGH_PRIORITY,RELIABLE_SEQUENCED,0,FALSE);
     } else {
      bsPickup.Write(i);
      pNetGame->GetRakClient()->RPC(RPC_PickedUpPickup,&bsPickup,HIGH_PRIORITY,RELIABLE_SEQUENCED,0,FALSE);
     }
    }
   } else if(m_iTimer[i]>0) m_iTimer[i]--;
  }
 }
}
void CPickupPoolMore::New(int iModel,float fX,float fY,float fZ,DWORD dwAmmo,WORD fromDeadPlayer)
{
 if(m_iPickupCount>=MAX_PICKUPS)return;
 int iPickup;
 for(iPickup=0;iPickup<MAX_PICKUPS;iPickup++) if(m_dwHnd[iPickup]==NULL) break;
 if(iPickup==MAX_PICKUPS)return;
 m_Pickups[iPickup].iModel=iModel;
 m_Pickups[iPickup].iType=4;
 m_Pickups[iPickup].fX=fX;
 m_Pickups[iPickup].fY=fY;
 m_Pickups[iPickup].fZ=fZ;
 m_dwHnd[iPickup]=pGame->CreateWeaponPickup(iModel,dwAmmo,fX,fY,fZ);
 m_iTimer[iPickup]=NULL;
 m_iNetworkIds[iPickup]=-1;
 m_droppedWeapon[iPickup].bDroppedWeapon=true;
 m_droppedWeapon[iPickup].fromPlayer=fromDeadPlayer;
 m_iPickupCount++;
}
