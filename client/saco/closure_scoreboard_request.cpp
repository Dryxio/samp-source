#include "main.h"
extern DWORD r5ScoreboardLastRequestTick;
extern const int r5RpcRequestScoresPings;
class R5ScoreboardRequestView {
public:
 RakClientInterface *client;
 void RequestScoresAndPings();
};
void R5ScoreboardRequestView::RequestScoresAndPings() {
 if(GetTickCount()-r5ScoreboardLastRequestTick>3000) {
  r5ScoreboardLastRequestTick=GetTickCount();
  RakNet::BitStream stream;
  if(client) client->RPC((char*)&r5RpcRequestScoresPings,&stream,HIGH_PRIORITY,RELIABLE,0,FALSE);
 }
}
