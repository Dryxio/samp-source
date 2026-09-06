// Partial views only. No allocations, constructors, virtual tables or sizeof owners.
#include "main.h"
#include "game/util.h"
#include <string.h>
#include <stddef.h>
extern CGame *pGame;
extern CNetGame *pNetGame;
struct RPCVehicleAppearanceView {
 BYTE unknown0[0x4c]; VEHICLE_TYPE *vehicle;
 BYTE unknown50[0x43]; char plate[32];
 void SetNumberPlate(char *text);
 void SetTireStatus(BYTE status);
};
typedef char RPCCarTireLayout[(offsetof(VEHICLE_TYPE,bCarWheelPopped)==0x5a5)?1:-1];
typedef char RPCBikeTireLayout[(offsetof(VEHICLE_TYPE,bBikeWheelPopped)==0x65c)?1:-1];
void RPCVehicleAppearanceView::SetNumberPlate(char *text) {
 strncpy(plate,text,32);
}
void RPCVehicleAppearanceView::SetTireStatus(BYTE status) {
 if(vehicle && GetVehicleSubtypeFromVehiclePtr(vehicle)==1) {
  if(status&1)vehicle->bCarWheelPopped[3]=1;else vehicle->bCarWheelPopped[3]=0;
  status>>=1;
  if(status&1)vehicle->bCarWheelPopped[2]=1;else vehicle->bCarWheelPopped[2]=0;
  status>>=1;
  if(status&1)vehicle->bCarWheelPopped[1]=1;else vehicle->bCarWheelPopped[1]=0;
  if(status&2)vehicle->bCarWheelPopped[0]=1;else vehicle->bCarWheelPopped[0]=0;
 } else if(vehicle && GetVehicleSubtypeFromVehiclePtr(vehicle)==2) {
  if(status&1)vehicle->bBikeWheelPopped[1]=1;else vehicle->bBikeWheelPopped[1]=0;
  if(status&2)vehicle->bBikeWheelPopped[0]=1;else vehicle->bBikeWheelPopped[0]=0;
 }
}
struct RPCVehicleAppearancePoolView {
 BYTE unknown0[0x1134]; RPCVehicleAppearanceView *vehicles[2000]; BOOL active[2000];
 RPCVehicleAppearanceView *GetAt(WORD id) {
  if(id>=2000)return NULL;
  if(active[id])return vehicles[id];
  return NULL;
 }
 BOOL GetSlotState(WORD id) { if(id>=2000)return FALSE;return active[id]; }
};
struct RPCVehicleAppearancePoolsView { RPCVehicleAppearancePoolView *vehicles; };
struct RPCVehicleAppearanceNetView { BYTE unknown0[0x3de]; RPCVehicleAppearancePoolsView *pools; };
void ScrVehicleNumberPlateTransfer(RPCParameters *rpcParams) {
 PCHAR Data=reinterpret_cast<PCHAR>(rpcParams->input);
 int bits=rpcParams->numberOfBitsOfData;
 RakNet::BitStream stream(Data,(bits/8)+1,false);
 WORD id;
 BYTE length=0;
 char text[33];memset(text,0,sizeof(text));
 stream.Read(id);stream.Read(length);
 if(length<=32) {
  stream.Read(text,length);
  RPCVehicleAppearancePoolView *pool=((RPCVehicleAppearanceNetView*)pNetGame)->pools->vehicles;
  if(pool && pool->GetSlotState(id))pool->vehicles[id]->SetNumberPlate(text);
 }
}
// Adapted0.2.5 RPC: R5 transmits one whole tire mask, no separate wheel index.
void ScrVehicleTireStatusTransfer(RPCParameters *rpcParams) {
 PCHAR Data=reinterpret_cast<PCHAR>(rpcParams->input);
 int bits=rpcParams->numberOfBitsOfData;
 RakNet::BitStream stream(Data,(bits/8)+1,false);
 WORD id;BYTE status;
 stream.Read(id);stream.Read(status);
 RPCVehicleAppearancePoolView *pool=((RPCVehicleAppearanceNetView*)pNetGame)->pools->vehicles;
 RPCVehicleAppearanceView *vehicle=pool->GetAt(id);
 if(pool && vehicle)vehicle->SetTireStatus(status);
}
const SCRIPT_COMMAND rpc_select_ped_interior={0x04bb,"i"};
const SCRIPT_COMMAND rpc_link_ped_interior={0x0860,"ii"};
const SCRIPT_COMMAND rpc_refresh_ped_streaming={0x04e4,"ff"};
struct RPCPedInteriorView {
 BYTE unknown0[0x44]; DWORD gtaId;
 BYTE unknown48[0x25c]; PED_TYPE *ped;
 BYTE unknown2a8[8]; BYTE playerNumber;
 void SetInterior(BYTE id,BOOL refresh);
};
// 0.2.5 method adapted: only local ped selects global interior/stream refresh.
void RPCPedInteriorView::SetInterior(BYTE id,BOOL refresh) {
 if(!ped)return;
 if(!playerNumber) {
  ScriptCommand(&rpc_select_ped_interior,id);
  ScriptCommand(&rpc_link_ped_interior,gtaId,id);
  if(refresh) {
   MATRIX4X4 matrix;
   ((CEntity*)this)->GetMatrix(&matrix);
   ScriptCommand(&rpc_refresh_ped_streaming,matrix.pos.X,matrix.pos.Y);
  }
 } else {
  ScriptCommand(&rpc_link_ped_interior,gtaId,id);
 }
}
void ScrPlayerInteriorTransfer(RPCParameters *rpcParams) {
 PCHAR Data=reinterpret_cast<PCHAR>(rpcParams->input);
 int bits=rpcParams->numberOfBitsOfData;
 RakNet::BitStream stream(Data,(bits/8)+1,false);
 BYTE interior;stream.Read(interior);
 ((RPCPedInteriorView*)pGame->FindPlayerPed())->SetInterior(interior,TRUE);
}
