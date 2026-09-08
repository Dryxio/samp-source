// Source0.2.5 facing-angle RPC, realR5BitStream char*ctor.
#include "main.h"
extern CGame *pGame;
void SetPlayerFacingAngleRPCTransfer(RPCParameters *rpcParams) {
 PCHAR data=(PCHAR)rpcParams->input;
 int bits=rpcParams->numberOfBitsOfData;
 RakNet::BitStream stream(data,bits/8+1,false);
 float angle;stream.Read(angle);
 pGame->FindPlayerPed()->ForceTargetRotation(angle);
}
