// Normal C++, 0.2.5 source adapted to original R5 verified layouts.
// Requires genuine CVehicle method declarations documented alongside this file.
#include "main.h"
#include "game/util.h"
extern CNetGame *pNetGame;
// Same existing view declaration/body as closure_rpc_vehicle_motion.cpp.
struct RPCVehicleMotionPoolView {
 int lastVehicleId; BYTE unknown4[0x1130]; CVehicle *vehicles[2000];
 BOOL active[2000]; VEHICLE_TYPE *nativeVehicles[2000];
 WORD FindIDFromGtaPtr(VEHICLE_TYPE *native);
 CVehicle *GetAt(WORD id) { if(id>=2000)return NULL; if(active[id])return vehicles[id]; return NULL; }
};
// Read-only partial view; never allocated or used as a complete pool object.
struct R5VehicleTrailerPoolView {
 BYTE unknown0[0x1134]; CVehicle *vehicles[2000]; BOOL active[2000];
 __declspec(dllexport) CVehicle *GetAt(WORD id);
 __declspec(dllexport) BOOL GetSlotState(WORD id);
};
struct R5VehicleTrailerPoolsView { R5VehicleTrailerPoolView *vehicles; };
struct R5VehicleTrailerNetView { BYTE unknown0[0x3de]; R5VehicleTrailerPoolsView *pools; };
CVehicle *CVehicle::GetTrailer() {
 if(!m_pVehicle)return NULL;
 VEHICLE_TYPE *trailer=(VEHICLE_TYPE*)m_pVehicle->dwTrailer;
 if(trailer && pNetGame) {
  R5VehicleTrailerPoolView *pool=((R5VehicleTrailerNetView*)pNetGame)->pools->vehicles;
  WORD id=((RPCVehicleMotionPoolView*)pool)->FindIDFromGtaPtr(trailer);
  if(id!=0xffff && pool->GetSlotState(id))return pool->GetAt(id);
 }
 return NULL;
}
void CVehicle::Add() {
 if(!IsAdded()) { CEntity::Add(); CVehicle *trailer=GetTrailer(); if(trailer)trailer->Add(); }
}
void CVehicle::Remove() {
 if(IsAdded()) { CVehicle *trailer=GetTrailer(); if(trailer)trailer->Remove(); CEntity::Remove(); }
}
void CVehicle::RemoveEveryoneFromVehicle() {
 if(!m_pVehicle)return;
 if(!GamePool_Vehicle_GetAt(m_dwGTAId))return;
 float x=m_pVehicle->entity.mat->pos.X;
 float y=m_pVehicle->entity.mat->pos.Y;
 float z=m_pVehicle->entity.mat->pos.Z;
 int actor=0;
 if(m_pVehicle->pDriver) {
  actor=GamePool_Ped_GetIndex(m_pVehicle->pDriver);
  ScriptCommand(&remove_actor_from_car_and_put_at,actor,x,y,z+2.0f);
 }
 for(int i=0;i<7;i++)if(m_pVehicle->pPassengers[i]) {
  actor=GamePool_Ped_GetIndex(m_pVehicle->pPassengers[i]);
  ScriptCommand(&remove_actor_from_car_and_put_at,actor,x,y,z+2.0f);
 }
}
// Native virtual destructor bridge follows actual 0.2.5 GamePrepareTrain.
void __stdcall GamePrepareTrain(VEHICLE_TYPE *vehicle) {
 if(!vehicle)return;
 PED_TYPE *driver=vehicle->pDriver;
 if(driver && driver->dwPedType!=0 && driver->dwPedType!=1) {
  DWORD nativeDriver=(DWORD)driver;
  _asm mov ecx,nativeDriver
  _asm mov ebx,[ecx]
  _asm push 1
  _asm call [ebx]
  vehicle->pDriver=0;
 }
}
