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
extern BYTE *pbyteCurrentPlayer;
void R5DeathPedView::SetDead() {
 if(gtaId && ped) {
  if(!IN_VEHICLE(ped) && ped->Tasks->pdwJumpJetPack && *(DWORD*)ped->Tasks->pdwJumpJetPack==0x8705C4)
   ((CPlayerPed*)this)->StopJetpack();
  ExtinguishFire();
  MATRIX4X4 matrix;
  ((CEntity*)this)->GetMatrix(&matrix);
  ((CEntity*)this)->TeleportTo(matrix.pos.X,matrix.pos.Y,matrix.pos.Z);
  ped->fHealth=0.0f;
  *pbyteCurrentPlayer=playerNumber;
  ScriptCommand(&kill_actor,gtaId);
  *pbyteCurrentPlayer=0;
 }
}
