// Full R5 rates: actual player pointer/LAN flag and real configured int rates.
#include "main.h"
#include <stddef.h>
extern CNetGame *pNetGame;
extern CGame *pGame;
// The two genuine adjacent initialized rate DWORDs form their whole section.
#pragma data_seg(".r5rates")
int r5NormalOnfootSendRate = 30;
int r5NormalIncarSendRate = 30;
#pragma data_seg()
#pragma pack(push,1)
class R5StatisticsLocalPlayerView
{
public:
 BYTE unknownPrefix[0x104];
 CPlayerPed *ped;
 int GetInCarSendRate();
 int GetOnFootSendRate();
};
struct R5StatisticsNetRateView
{
 RakClientInterface *client;
 BYTE unknownPrefix[0x239-4];
 BOOL lanMode;
};
#pragma pack(pop)
typedef char R5StatsPedOffset[(offsetof(R5StatisticsLocalPlayerView,ped)==0x104)?1:-1];
typedef char R5StatsLanOffset[(offsetof(R5StatisticsNetRateView,lanMode)==0x239)?1:-1];
int R5StatisticsLocalPlayerView::GetInCarSendRate()
{
 if(ped)
 {
  if(((R5StatisticsNetRateView*)pNetGame)->lanMode) return 15;
  return pGame->FUNC_100A00F0()+r5NormalIncarSendRate;
 }
 return 1000;
}
int R5StatisticsLocalPlayerView::GetOnFootSendRate()
{
 if(ped)
 {
  if(((R5StatisticsNetRateView*)pNetGame)->lanMode) return 15;
  return pGame->FUNC_100A00F0()+r5NormalOnfootSendRate;
 }
 return 1000;
}
