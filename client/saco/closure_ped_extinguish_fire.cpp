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
void R5DeathPedView::ExtinguishFire() {
 if(ped) {
  DWORD fire=((R5NativePedFireView*)ped)->fire;
  if(fire) { _asm mov ecx,fire
             _asm mov eax,0x5393F0
             _asm call eax }
  if(IN_VEHICLE(ped)) {
   VEHICLE_TYPE *vehicle=(VEHICLE_TYPE*)ped->pVehicle;
   if(vehicle) {
    fire=((R5NativeVehicleFireView*)vehicle)->fire;
    if(fire) { _asm mov ecx,fire
               _asm mov eax,0x5393F0
               _asm call eax }
   }
  }
 }
}
