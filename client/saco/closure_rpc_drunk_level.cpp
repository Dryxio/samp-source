#include "main.h"
extern CGame *pGame;
#pragma pack(push,1)
class R5DrunkLevelPedView {
 BYTE prefix[0x2c9]; int level;
public:
 void SetDrunkLevel(int value);
};
#pragma pack(pop)
void SetPlayerDrunkLevelRPCTransfer(RPCParameters *rpcParams) {
 PCHAR data=(PCHAR)rpcParams->input;
 int bits=rpcParams->numberOfBitsOfData;
 RakNet::BitStream stream(data,bits/8+1,false);
 int level;stream.Read(level);
 CPlayerPed *ped=pGame->FindPlayerPed();
 if(ped)((R5DrunkLevelPedView*)ped)->SetDrunkLevel(level);
}
