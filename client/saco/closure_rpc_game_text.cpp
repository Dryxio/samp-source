#include "main.h"
#include <string.h>
extern CGame *pGame;
extern unsigned char *szGameTextMessage;
// Existing dashr base implementation. Explicit symbolic native call retained.
void CGame::DisplayGameText(char *szStr,int iTime,int iSize) {
 if(iSize>200)return;
 ScriptCommand(&text_clear_all);
 memset(szGameTextMessage,0,sizeof(szGameTextMessage)); // observed four bytes
 strncpy((char*)szGameTextMessage,szStr,512);
 unsigned char *str=szGameTextMessage;
 while(*str!=0) {
  if(*str>=1 && *str<' ' && *str!='\n')*str=' ';
  str++;
 }
 _asm push iSize
 _asm push iTime
 _asm push szGameTextMessage
 _asm mov eax,0x69F2B0
 _asm call eax
 _asm add esp,12
}
// 0.2.5 RPC adapted to R5 buffer256, zero initialization, bounds1..200.
void ScrDisplayGameTextTransfer(RPCParameters *rpcParams) {
 PCHAR Data=reinterpret_cast<PCHAR>(rpcParams->input);
 int bits=rpcParams->numberOfBitsOfData;
 RakNet::BitStream stream(Data,(bits/8)+1,false);
 char message[256];memset(message,0,sizeof(message));
 int type,time,length;
 stream.Read(type);stream.Read(time);stream.Read(length);
 if(length>=1 && length<=200) {
  stream.Read(message,length);
  message[length]=0;
  pGame->DisplayGameText(message,time,type);
 }
}
