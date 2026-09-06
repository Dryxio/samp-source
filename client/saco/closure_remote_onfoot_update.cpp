#include "main.h"
#include "game/util.h"
#include <stddef.h>
#pragma pack(push,1)
struct R5PedExitTaskView {
 BYTE unknown0[0x44]; DWORD gtaId;
 BYTE unknown48[0x25c]; PED_TYPE *ped;
 BOOL IsExitingVehicle();
};
// Complete 68-byte R5 on-foot network record.
struct R5RemoteOnFootSync {
 WORD leftRight,upDown,keys;
 VECTOR position;
 float quaternion[4];
 BYTE health,armour,weaponAndFlags,specialAction;
 VECTOR moveSpeed;
 VECTOR surfingOffset;
 WORD surfingVehicle;
 int animation;
};
struct R5RemoteOnFootUpdateView {
 BYTE unknown0[0xc]; BYTE afk;
 BYTE unknownD[0xb8]; R5RemoteOnFootSync onFoot;
 BYTE unknown109; BYTE state;
 BYTE unknown10B[0xa1]; float reportedArmour,reportedHealth;
 BYTE unknown1B4[4]; BYTE updateType;
 DWORD lastReceivedTick,lastTimestamp;
 BYTE unknown1C1[0x1c]; CPlayerPed *ped;
 CVehicle *vehicle;
 void EnsureVehicleExit();
 void StoreOnFootSync(R5RemoteOnFootSync *sync,DWORD timestamp);
};
#pragma pack(pop)
typedef char ExitPedOffset[(offsetof(R5PedExitTaskView,ped)==0x2a4)?1:-1];
typedef char OnFootSize[(sizeof(R5RemoteOnFootSync)==68)?1:-1];
typedef char OnFootHealth[(offsetof(R5RemoteOnFootSync,health)==0x22)?1:-1];
typedef char OnFootStored[(offsetof(R5RemoteOnFootUpdateView,onFoot)==0xc5)?1:-1];
typedef char OnFootState[(offsetof(R5RemoteOnFootUpdateView,state)==0x10a)?1:-1];
typedef char OnFootHealthStored[(offsetof(R5RemoteOnFootUpdateView,reportedHealth)==0x1b0)?1:-1];
typedef char OnFootTimestamp[(offsetof(R5RemoteOnFootUpdateView,lastTimestamp)==0x1bd)?1:-1];
typedef char OnFootPed[(offsetof(R5RemoteOnFootUpdateView,ped)==0x1dd)?1:-1];
BOOL R5PedExitTaskView::IsExitingVehicle()
{
 if(GamePool_Ped_GetAt(gtaId) && ped && ped->Tasks && ped->Tasks->pdwJumpJetPack)
  if(GetTaskTypeFromTask(ped->Tasks->pdwJumpJetPack)==704)return TRUE;
 return FALSE;
}
void R5RemoteOnFootUpdateView::EnsureVehicleExit()
{
 MATRIX4X4 matrix;
 if(ped && ped->IsInVehicle()) {
  ped->GetMatrix(&matrix);
  ped->RemoveFromVehicleAndPutAt(matrix.pos.X,matrix.pos.Y,matrix.pos.Z);
  vehicle=NULL;
 }
}
void R5RemoteOnFootUpdateView::StoreOnFootSync(R5RemoteOnFootSync *sync,DWORD timestamp)
{
 vehicle=NULL;
 if(timestamp && (int)(timestamp-lastTimestamp)<0)return;
 lastTimestamp=timestamp;
 memcpy(&onFoot,sync,sizeof(R5RemoteOnFootSync));
 reportedHealth=(float)sync->health;
 reportedArmour=(float)sync->armour;
 updateType=16;
 lastReceivedTick=GetTickCount();
 afk=sync->specialAction;
 if(onFoot.animation & 0x80000000)onFoot.animation=0;
 if(ped && ped->IsInVehicle() && afk!=3 && afk!=4 && !((R5PedExitTaskView*)ped)->IsExitingVehicle())EnsureVehicleExit();
 if(state!=17)state=17;
}
