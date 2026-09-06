#include "main.h"
extern CNetGame *pNetGame;
extern const int r5RpcChat;
void CLocalPlayer::Say(PCHAR szText)
{
 BYTE byteTextLen=strlen(szText);
 RakNet::BitStream bsSend;
 bsSend.Write(byteTextLen);
 bsSend.Write(szText,byteTextLen);
 pNetGame->GetRakClient()->RPC((char*)&r5RpcChat,&bsSend,HIGH_PRIORITY,RELIABLE,0,false);
}
