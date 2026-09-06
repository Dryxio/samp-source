#include "main.h"
#include "game/util.h"
#include <stddef.h>
extern CNetGame *pNetGame;
#pragma pack(push,1)
struct RPCVehicleParamsView {
 BYTE prefix[0x44]; DWORD gtaId; CVehicle *trailer; VEHICLE_TYPE *native;
 BYTE gap50[0x10]; BOOL doorsLocked; BYTE objective; BOOL specialMarker;
 void SetDoorState(int state);
 BOOL RemoveComponent(WORD component);
};
struct RPCVehicleParamsPoolView {
 BYTE prefix[0x1134]; RPCVehicleParamsView *vehicles[2000]; BOOL slots[2000];
 BYTE gap4FB4[0x6D60]; BOOL active[2000];
 BOOL GetSlotState(WORD id) { if(id>=2000)return FALSE; return slots[id]; }
 RPCVehicleParamsView *GetAt(WORD id) { if(id>=2000)return NULL; if(slots[id])return vehicles[id]; return NULL; }
 void AssignSpecialParamsToVehicle(WORD id,BYTE objective,BYTE doors);
};
struct RPCVehicleParamsPoolsView { RPCVehicleParamsPoolView *vehicles; };
struct RPCVehicleParamsNetView { BYTE prefix[0x3DE]; RPCVehicleParamsPoolsView *pools; RPCVehicleParamsPoolView *GetVehiclePool() {return pools->vehicles;} };
#pragma pack(pop)
typedef char ParamsDoorsOffset[(offsetof(RPCVehicleParamsView,doorsLocked)==0x60)?1:-1];
typedef char ParamsObjectiveOffset[(offsetof(RPCVehicleParamsView,objective)==0x64)?1:-1];
typedef char ParamsActiveOffset[(offsetof(RPCVehicleParamsPoolView,active)==0xBD14)?1:-1];
typedef char NativeDoorOffset[(offsetof(VEHICLE_TYPE,dwDoorsLocked)==0x4F8)?1:-1];

void ScrVehicleParamsTransfer(RPCParameters *rpcParams)
{
 PCHAR Data=reinterpret_cast<PCHAR>(rpcParams->input);
 int bits=rpcParams->numberOfBitsOfData;
 RakNet::BitStream stream(Data,(bits/8)+1,false);
 WORD id; BYTE objective,doors;
 stream.Read(id);stream.Read(objective);stream.Read(doors);
 RPCVehicleParamsPoolView *pool=((RPCVehicleParamsNetView*)pNetGame)->GetVehiclePool();
 if(pool)pool->AssignSpecialParamsToVehicle(id,objective,doors);
}
void ScrRemoveComponentTransfer(RPCParameters *rpcParams)
{
 PCHAR Data=reinterpret_cast<PCHAR>(rpcParams->input);
 int bits=rpcParams->numberOfBitsOfData;
 RakNet::BitStream stream(Data,(bits/8)+1,false);
 RPCVehicleParamsPoolView *pool=((RPCVehicleParamsNetView*)pNetGame)->GetVehiclePool();
 if(!pool)return;
 WORD id,component;
 stream.Read(id);stream.Read(component);
 if(pool->GetSlotState(id))pool->GetAt(id)->RemoveComponent(component);
}
