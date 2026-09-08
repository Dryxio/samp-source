#include "main.h"
extern CNetGame *pNetGame;
struct R5VehicleDamageView {
 BYTE unknown0[0x4c]; VEHICLE_TYPE *vehicle;
 void UpdateDamageStatus(DWORD panels,DWORD doors,BYTE lights);
};
struct RPCVehicleAppearanceView {
 BYTE unknown0[0x4c]; VEHICLE_TYPE *vehicle;
 BYTE unknown50[0x43]; char plate[32];
 void SetNumberPlate(char *text);
 void SetTireStatus(BYTE status);
};
struct R5VehicleDamagePoolView {
 BYTE unknown0[0x1134]; R5VehicleDamageView *vehicles[2000]; BOOL active[2000];
 R5VehicleDamageView *GetAt(WORD id) {
  if(id>=2000)return NULL;
  if(active[id])return vehicles[id];
  return NULL;
 }
};
struct R5VehicleDamagePoolsView { R5VehicleDamagePoolView *vehicles; };
struct R5VehicleDamageNetView { BYTE unknown0[0x3de]; R5VehicleDamagePoolsView *pools; };
void DamageVehicleRPCTransfer(RPCParameters *rpcParams) {
 PCHAR Data=(PCHAR)rpcParams->input;
 int iBitLength=rpcParams->numberOfBitsOfData;
 RakNet::BitStream bsData(Data,iBitLength/8+1,false);
 WORD VehicleID;
 DWORD dwPanels,dwDoors;
 BYTE byteLights,byteTires;
 bsData.Read(VehicleID);bsData.Read(dwPanels);bsData.Read(dwDoors);bsData.Read(byteLights);bsData.Read(byteTires);
 R5VehicleDamageView *pVehicle=((R5VehicleDamageNetView*)pNetGame)->pools->vehicles->GetAt(VehicleID);
 if(pVehicle) {
  pVehicle->UpdateDamageStatus(dwPanels,dwDoors,byteLights);
  ((RPCVehicleAppearanceView*)pVehicle)->SetTireStatus(byteTires);
 }
}
