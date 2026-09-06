#include "main.h"
#include "game/util.h"
extern CNetGame *pNetGame;
extern BOOL r5TrailerDetachInProgress;
extern const SCRIPT_COMMAND r5PutTrailerOnCab;
extern const SCRIPT_COMMAND r5DetachTrailerFromCab;
struct RPCTrailerPoolView {
 BYTE beforeVehicles[0x1134];
 CVehicle *vehicles[2000];
 BOOL active[2000];
 CVehicle *GetAt(WORD id) {
  if(id>=2000)return NULL;
  if(active[id])return vehicles[id];
  return NULL;
 }
};
struct RPCTrailerPoolsView { RPCTrailerPoolView *vehicles; };
struct RPCTrailerNetView { BYTE beforePools[0x3DE]; RPCTrailerPoolsView *pools; RPCTrailerPoolView *GetVehiclePool() { return pools->vehicles; } };
void ScrAttachTrailerToVehicleTransfer(RPCParameters *rpcParams)
{
 PCHAR Data=reinterpret_cast<PCHAR>(rpcParams->input);
 int iBitLength=rpcParams->numberOfBitsOfData;
 WORD TrailerID,VehicleID;
 RakNet::BitStream bsData(Data,(iBitLength/8)+1,false);
 if(!((RPCTrailerNetView*)pNetGame)->GetVehiclePool())return;
 bsData.Read(TrailerID);
 bsData.Read(VehicleID);
 CVehicle *pTrailer=((RPCTrailerNetView*)pNetGame)->GetVehiclePool()->GetAt(TrailerID);
 CVehicle *pVehicle=((RPCTrailerNetView*)pNetGame)->GetVehiclePool()->GetAt(VehicleID);
 if(pTrailer && pVehicle) {
  pVehicle->SetTrailer(pTrailer);
  pVehicle->AttachTrailer();
 }
}
void ScrDetachTrailerFromVehicleTransfer(RPCParameters *rpcParams)
{
 PCHAR Data=reinterpret_cast<PCHAR>(rpcParams->input);
 int iBitLength=rpcParams->numberOfBitsOfData;
 if(!((RPCTrailerNetView*)pNetGame)->GetVehiclePool())return;
 WORD VehicleID;
 RakNet::BitStream bsData(Data,(iBitLength/8)+1,false);
 bsData.Read(VehicleID);
 CVehicle *pVehicle=((RPCTrailerNetView*)pNetGame)->GetVehiclePool()->GetAt(VehicleID);
 if(pVehicle) {
  pVehicle->DetachTrailer();
  pVehicle->SetTrailer(NULL);
 }
}
