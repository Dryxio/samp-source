#include "main.h"
extern CNetGame *pNetGame;
extern CGame *pGame;
struct RPCVehicleMotionPoolView {
 int lastVehicleId;
 BYTE unknown4[0x1130];
 CVehicle *vehicles[2000];
 BOOL active[2000];
 VEHICLE_TYPE *nativeVehicles[2000];
 WORD FindIDFromGtaPtr(VEHICLE_TYPE *native);
 CVehicle *GetAt(WORD id) {
  if(id>=2000)return NULL;
  if(active[id])return vehicles[id];
  return NULL;
 }
};
struct R5VehicleGtaIdPoolView { BYTE prefix[0x4fb4]; VEHICLE_TYPE *nativeVehicles[2000]; int FindGtaIDFromID(int id); };
struct R5VehicleGtaIdPoolsView { R5VehicleGtaIdPoolView *vehicles; };
struct R5VehicleGtaIdNetView { BYTE prefix[0x3de]; R5VehicleGtaIdPoolsView *pools; R5VehicleGtaIdPoolView *GetVehiclePool() { return pools->vehicles; } };
void ScrPutPlayerInVehicleTransfer(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData(Data,(iBitLength/8)+1,false);
	VEHICLEID vehicleid;
	BYTE seatid;
	bsData.Read(vehicleid);
	bsData.Read(seatid);
	int iVehicleIndex = ((R5VehicleGtaIdNetView*)pNetGame)->GetVehiclePool()->FindGtaIDFromID(vehicleid);
	CVehicle *pVehicle = ((RPCVehicleMotionPoolView*)((R5VehicleGtaIdNetView*)pNetGame)->GetVehiclePool())->GetAt(vehicleid);

	if(iVehicleIndex && pVehicle) {
		 pGame->FindPlayerPed()->PutDirectlyInVehicle(iVehicleIndex, seatid);
	}
}
void ScrTogglePlayerControllableTransfer(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData(Data,(iBitLength/8)+1,false);
	BYTE byteControllable;
	bsData.Read(byteControllable);
	pGame->FindPlayerPed()->TogglePlayerControllable((int)byteControllable);
}
