// Diagnostic proposal only, requires exact comparison and reviewed target closure.
#include "main.h"
extern CGame *pGame;
extern BOOL __stdcall IsPickupModelValid(int model);
const SCRIPT_COMMAND create_pickup_r5 = {0x0213,"iifffv"};
class CGamePickupMore {
public:
 BOOL IsModelLoaded(int model) { if(model>20000 || model<0)return TRUE; return ScriptCommand(&is_model_available,model); }
 void RequestModel(int model) { ScriptCommand(&request_model,model); }
 void LoadRequestedModels() { ScriptCommand(&load_requested_models); }
 DWORD CreatePickup(int model,int type,float x,float y,float z,int *index);
};
DWORD CGamePickupMore::CreatePickup(int model,int type,float x,float y,float z,int *index)
{
 DWORD hnd;
 if(!IsPickupModelValid(model)) model=18631;
 if(!IsModelLoaded(model)) {
  RequestModel(model);
  LoadRequestedModels();
  while(!IsModelLoaded(model)) Sleep(1);
 }
 ScriptCommand(&create_pickup_r5,model,type,x,y,z,&hnd);
 int result=(int)((hnd&0xffff)*32);
 if(result) result/=32;
 if(index)*index=result;
 return hnd;
}
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
void CPickupPoolCreate::New(R5PickupRecordMore *pPickup,int iPickup)
{
 int networkIndex;
 if(m_iPickupCount>=MAX_PICKUPS || iPickup<0 || iPickup>=MAX_PICKUPS)return;
 if(m_dwHnd[iPickup]!=NULL)ScriptCommand(&destroy_pickup,m_dwHnd[iPickup]);
 memcpy(&m_Pickups[iPickup],pPickup,sizeof(R5PickupRecordMore));
 m_droppedWeapon[iPickup].bDroppedWeapon=false;
 m_dwHnd[iPickup]=((CGamePickupMore*)pGame)->CreatePickup(pPickup->iModel,pPickup->iType,pPickup->fX,pPickup->fY,pPickup->fZ,&networkIndex);
 m_iNetworkIds[iPickup]=networkIndex;
 m_iPickupCount++;
}
