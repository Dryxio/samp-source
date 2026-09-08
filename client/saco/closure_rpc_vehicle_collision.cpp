#include "main.h"
extern CNetGame *pNetGame;
#pragma pack(push,1)
struct R5VehicleCollisionNetView { BYTE prefix[0x233]; bool collision; };
#pragma pack(pop)
void SetVehicleCollisionRPCTransfer(RPCParameters *rpcParams) {
 PCHAR data=(PCHAR)rpcParams->input;
 int bits=rpcParams->numberOfBitsOfData;
 RakNet::BitStream stream(data,bits/8+1,false);
 bool enabled=false;
 stream.Read(enabled);
 if(pNetGame)((R5VehicleCollisionNetView*)pNetGame)->collision=enabled;
}
