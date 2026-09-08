#include "main.h"
extern CNetGame *pNetGame;
#pragma pack(push,1)
struct R5NetFlag232View { BYTE prefix[0x232]; bool collision; };
#pragma pack(pop)
void SetNetGameFlag232RPCTransfer(RPCParameters *rpcParams) {
 PCHAR data=(PCHAR)rpcParams->input;
 int bits=rpcParams->numberOfBitsOfData;
 RakNet::BitStream stream(data,bits/8+1,false);
 bool enabled=false;
 stream.Read(enabled);
 if(pNetGame)((R5NetFlag232View*)pNetGame)->collision=enabled;
}
