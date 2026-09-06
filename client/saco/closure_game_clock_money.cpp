// Existing base CGame bodies with verified helper bodies expanded for Ob1.
#include "main.h"
#include "game/util.h"
void CGame::EnableClock(BYTE byteClock)
{
 BYTE byteClockData[]={'%','0','2','d',':','%','0','2','d',0};
 UnFuck(0x859A6C,10);
 if(byteClock) {
  UnFuck(0x52CF10,1);
  *(PBYTE)0x52CF10=0x56;
  field_69=TRUE;
  memcpy((PVOID)0x859A6C,byteClockData,10);
 } else {
  UnFuck(0x52CF10,1);
  *(PBYTE)0x52CF10=0xC3;
  field_69=FALSE;
  memset((PVOID)0x859A6C,0,10);
 }
}
void CGame::ResetLocalMoney()
{
 int iMoney=*(int*)0xB7CE50;
 if(!iMoney)return;
 if(iMoney<0) ScriptCommand(&add_to_player_money,0,abs(iMoney));
 else ScriptCommand(&add_to_player_money,0,-iMoney);
}
