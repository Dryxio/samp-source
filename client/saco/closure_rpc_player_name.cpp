// Direct0.2.5 handler using existing R5 CPlayerPool/CNetPlayer std::string owners.
// MSVC2003 xstring providers must be emitted and fully matched, including EH.
// No replacement std::string layout, opaque allocation or copied code bytes.
#include "main.h"
#include <string>
extern CNetGame *pNetGame;
typedef char RPCNameStringSize[(sizeof(std::string)==28)?1:-1];
void ScrSetPlayerNameTransfer(RPCParameters *rpcParams) {
 PCHAR Data=reinterpret_cast<PCHAR>(rpcParams->input);
 int bits=rpcParams->numberOfBitsOfData;
 PLAYERID playerId;
 BYTE byteNickLen;
 char newName[MAX_PLAYER_NAME+1];
 BYTE success;
 RakNet::BitStream stream(Data,(bits/8)+1,false);
 CPlayerPool *pool=pNetGame->GetPlayerPool();
 stream.Read(playerId);
 stream.Read(byteNickLen);
 if(byteNickLen>MAX_PLAYER_NAME)return;
 stream.Read(newName,byteNickLen);
 stream.Read(success);
 newName[byteNickLen]=0;
 if(success==1)pool->SetPlayerName(playerId,newName);
 if(pool->GetLocalPlayerID()==playerId)pool->SetLocalPlayerName(newName);
}
