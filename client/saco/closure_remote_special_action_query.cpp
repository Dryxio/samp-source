#include "main.h"
#include "game/util.h"
#include <stddef.h>
#pragma pack(push,1)
struct R5PedEntryTaskView {
 BYTE unknown0[0x44]; DWORD gtaId;
 BYTE unknown48[0x25c]; PED_TYPE *ped;
 BOOL GetVehicleEntryTask();
};
struct R5RemoteSpecialActionView {
 BYTE unknown0[0x1c5]; int specialAction;
 BYTE unknown1C9[0x14]; CPlayerPed *ped;
 int GetSpecialAction();
};
struct R5PedExitTaskView {
 BYTE unknown0[0x44]; DWORD gtaId;
 BYTE unknown48[0x25c]; PED_TYPE *ped;
 BOOL IsExitingVehicle();
};
#pragma pack(pop)
typedef char EntryTaskPed[(offsetof(R5PedEntryTaskView,ped)==0x2a4)?1:-1];
typedef char RemoteSpecialPed[(offsetof(R5RemoteSpecialActionView,ped)==0x1dd)?1:-1];
BOOL R5PedEntryTaskView::GetVehicleEntryTask()
{
 if(GamePool_Ped_GetAt(gtaId) && ped && ped->Tasks && ped->Tasks->pdwJumpJetPack) {
  int task=GetTaskTypeFromTask(ped->Tasks->pdwJumpJetPack);
  if(task==700 || task==712)return 2;
  if(task==701 || task==713)return 1;
 }
 return 0;
}
int R5RemoteSpecialActionView::GetSpecialAction()
{
 if(ped && ped->IsAdded() && !((R5PedEntryTaskView*)ped)->GetVehicleEntryTask() && !((R5PedExitTaskView*)ped)->IsExitingVehicle())return specialAction;
 return 0;
}
