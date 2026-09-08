#include "main.h"
#include "game/util.h"
#pragma pack(push,1)
class R5DeathPedView {
 BYTE prefix[0x44]; DWORD gtaId;
 BYTE gap48[0x25c]; PED_TYPE *ped;
 BYTE gap2a8[8]; BYTE playerNumber;
public:
 void ExtinguishFire();
 void SetDead();
};
struct R5NativePedFireView { BYTE prefix[0x730]; DWORD fire; };
struct R5NativeVehicleFireView { BYTE prefix[0x490]; DWORD fire; };
#pragma pack(pop)
#pragma pack(push,1)
struct R5RemoteDeathView {
 BYTE prefix[0x10a]; BYTE state;
 BYTE gap10b[0xd2]; CPlayerPed *ped;
 void HandleDeath();
};
#pragma pack(pop)
void R5RemoteDeathView::HandleDeath() {
 if(ped) {
  ped->SetKeys(0,0,0);
  ((R5DeathPedView*)ped)->SetDead();
 }
 if(state!=32)state=32;
 ((CRemotePlayer*)this)->ResetAllSyncAttributes();
}
