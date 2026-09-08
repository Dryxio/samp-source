#include "main.h"
extern CChatWindow *pChatWindow;
extern CNetGame *pNetGame;
class R5ChatInfoView {
    BYTE reserved0[0x126];
    DWORD infoColor;
public:
    void AddInfoMessage(char *format,...);
};

// Source025 ConnectionRejected with exact R5 strings and one-message reasons3/4.
void ConnectionRejectedRPCTransfer(RPCParameters *rpcParams) {
 PCHAR data=(PCHAR)rpcParams->input;
 int bits=rpcParams->numberOfBitsOfData;
 RakNet::BitStream stream(data,bits/8+1,false);
 BYTE reason;stream.Read(reason);
 if(reason==1) {
  ((R5ChatInfoView*)pChatWindow)->AddInfoMessage("CONNECTION REJECTED: Incorrect Version.");
 } else if(reason==2) {
  ((R5ChatInfoView*)pChatWindow)->AddInfoMessage("CONNECTION REJECTED: Unacceptable NickName");
  ((R5ChatInfoView*)pChatWindow)->AddInfoMessage("Please choose another nick between and 3-20 characters");
  ((R5ChatInfoView*)pChatWindow)->AddInfoMessage("Please use only a-z, A-Z, 0-9");
  ((R5ChatInfoView*)pChatWindow)->AddInfoMessage("Use /quit to exit or press ESC and select Quit Game");
 } else if(reason==3) {
  ((R5ChatInfoView*)pChatWindow)->AddInfoMessage("CONNECTION REJECTED: Bad mod version.");
 } else if(reason==4) {
  ((R5ChatInfoView*)pChatWindow)->AddInfoMessage("CONNECTION REJECTED: Unable to allocate a player slot.");
 }
 pNetGame->GetRakClient()->Disconnect(500);
}
