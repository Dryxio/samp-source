#include "main.h"
#include "game/util.h"
#include <stddef.h>
extern CNetGame *pNetGame;
#pragma pack(push,1)
struct RPCVehicleParamsView {
 BYTE prefix[0x44]; DWORD gtaId; CVehicle *trailer; VEHICLE_TYPE *native;
 BYTE gap50[0x10]; BOOL doorsLocked; BYTE objective; BOOL specialMarker;
 void SetDoorState(int state);
 BOOL RemoveComponent(WORD component);
};
struct RPCVehicleParamsPoolView {
 BYTE prefix[0x1134]; RPCVehicleParamsView *vehicles[2000]; BOOL slots[2000];
 BYTE gap4FB4[0x6D60]; BOOL active[2000];
 BOOL GetSlotState(WORD id) { if(id>=2000)return FALSE; return slots[id]; }
 RPCVehicleParamsView *GetAt(WORD id) { if(id>=2000)return NULL; if(slots[id])return vehicles[id]; return NULL; }
 void AssignSpecialParamsToVehicle(WORD id,BYTE objective,BYTE doors);
};
struct RPCVehicleParamsPoolsView { RPCVehicleParamsPoolView *vehicles; };
struct RPCVehicleParamsNetView { BYTE prefix[0x3DE]; RPCVehicleParamsPoolsView *pools; RPCVehicleParamsPoolView *GetVehiclePool() {return pools->vehicles;} };
#pragma pack(pop)
typedef char ParamsDoorsOffset[(offsetof(RPCVehicleParamsView,doorsLocked)==0x60)?1:-1];
typedef char ParamsObjectiveOffset[(offsetof(RPCVehicleParamsView,objective)==0x64)?1:-1];
typedef char ParamsActiveOffset[(offsetof(RPCVehicleParamsPoolView,active)==0xBD14)?1:-1];
typedef char NativeDoorOffset[(offsetof(VEHICLE_TYPE,dwDoorsLocked)==0x4F8)?1:-1];

extern const SCRIPT_COMMAND r5RemoveVehicleComponent;
void RPCVehicleParamsView::SetDoorState(int state)
{
 if(state) { native->dwDoorsLocked=2; doorsLocked=TRUE; }
 else { native->dwDoorsLocked=0; doorsLocked=FALSE; }
}
void RPCVehicleParamsPoolView::AssignSpecialParamsToVehicle(WORD id,BYTE objective,BYTE doors)
{
 if(!GetSlotState(id))return;
 RPCVehicleParamsView *vehicle=vehicles[id];
 if(vehicle && active[id]) {
  if(objective) {vehicle->objective=1;vehicle->specialMarker=FALSE;}
  vehicle->SetDoorState(doors);
 }
}
// Complete symbolic exception after three C++ forms preserved identical61-byte
// behavior but scheduled the shared failure block after success instead of before.
__declspec(naked) BOOL RPCVehicleParamsView::RemoveComponent(WORD component)
{
 enum { NativeVehicle=offsetof(RPCVehicleParamsView,native), GtaId=offsetof(RPCVehicleParamsView,gtaId), ComponentArgument=2*sizeof(DWORD), ScriptArgumentBytes=3*sizeof(DWORD), ArgumentBytes=sizeof(DWORD) };
 __asm {
  push esi
  mov esi, ecx
  mov eax, dword ptr [esi+NativeVehicle]
  test eax, eax
  je unavailable
  mov eax, dword ptr [esi+GtaId]
  push eax
  call GamePool_Vehicle_GetAt
  test eax, eax
  jne remove_component
 unavailable:
  xor eax, eax
  pop esi
  ret ArgumentBytes
 remove_component:
  movzx ecx, word ptr [esp+ComponentArgument]
  mov edx, dword ptr [esi+GtaId]
  push ecx
  push edx
  push offset r5RemoveVehicleComponent
  call ScriptCommand
  add esp, ScriptArgumentBytes
  mov eax, TRUE
  pop esi
  ret ArgumentBytes
 }
}
