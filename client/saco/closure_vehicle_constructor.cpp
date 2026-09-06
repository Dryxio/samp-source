// Normal C++ reconstruction of R5 B83D0/1308 from genuine 0.2.5 CVehicle.
#include "main.h"
#include "game/util.h"
extern CGame *pGame;
extern CChatWindow *pChatWindow;
extern BOOL r5AllowVehicleCreation;
extern DWORD r5LastCreatedVehicleId;
extern VEHICLE_TYPE *r5LastCreatedVehicle;
extern "C" const SCRIPT_COMMAND rpc_set_car_z_angle;
extern const SCRIPT_COMMAND r5CreateVehicle, r5VehicleGasTankExplosion;
extern const SCRIPT_COMMAND r5VehicleHydraulics, r5VehicleTiresVulnerable, r5CreateTrain;
void __stdcall GamePrepareTrain(VEHICLE_TYPE *vehicle);
// Verified individual members in existing CVehicle gaps; no invented full view.
#define VEH_DWORD(offset) (*(DWORD*)((BYTE*)this+(offset)))
#define VEH_BYTE(offset) (*(BYTE*)((BYTE*)this+(offset)))
CVehicle::CVehicle(int model,float x,float y,float z,float rotation,BOOL keepModel,int creationFlag) {
 DWORD vehicleId=0;
 m_pVehicle=0; m_dwGTAId=0; m_pTrailer=0;
 if(model!=538 && model!=537 && model!=570 && model!=569 && model!=449) {
  if(!pGame->IsModelLoaded(model)) {
   pGame->RequestModel(model,2); pGame->LoadRequestedModels();
   while(!pGame->IsModelLoaded(model))Sleep(0);
  }
  if(creationFlag) { r5AllowVehicleCreation=TRUE; VEH_DWORD(0x8b)=TRUE; }
  else VEH_DWORD(0x8b)=FALSE;
  ScriptCommand(&r5CreateVehicle,model,x,y,z,&vehicleId);
  ScriptCommand(&rpc_set_car_z_angle,vehicleId,rotation);
  ScriptCommand(&r5VehicleGasTankExplosion,vehicleId,0);
  ScriptCommand(&r5VehicleHydraulics,vehicleId,0);
  ScriptCommand(&r5VehicleTiresVulnerable,vehicleId,0);
  m_pVehicle=GamePool_Vehicle_GetAt(vehicleId);
  m_pEntity=(ENTITY_TYPE*)m_pVehicle;
  m_dwGTAId=vehicleId;
  if(!m_pVehicle) {
   pChatWindow->AddDebugMessage("Warning: couldn't create vehicle type: %u",model);
   return;
  }
  r5LastCreatedVehicleId=vehicleId;
  m_pVehicle->dwDoorsLocked=0;
  *(DWORD*)((BYTE*)m_pVehicle+0x4b0)=0; // native state4B0, meaning unproven
  VEH_DWORD(0x5c)=FALSE;
  MATRIX4X4 matrix;
  GetMatrix(&matrix);
  matrix.pos.Z=(GetVehicleSubtype()!=2 && GetVehicleSubtype()!=6) ? (double)(z+0.25f) : (double)z;
  matrix.pos.X=x; matrix.pos.Y=y;
  SetMatrix(matrix);
  VEH_DWORD(0x87)=keepModel;
 } else if(model==538 || model==537 || model==449) {
  if(model==538)model=5;
  else if(model==537)model=3;
  else if(model==449)model=9;
  DWORD direction=0;
  if(rotation>180.0f)direction=1;
  pGame->RequestModel(538,2); pGame->RequestModel(570,2);
  pGame->RequestModel(537,2); pGame->RequestModel(569,2); pGame->RequestModel(449,2);
  pGame->LoadRequestedModels();
  while(!pGame->IsModelLoaded(538))Sleep(0);
  while(!pGame->IsModelLoaded(570))Sleep(0);
  while(!pGame->IsModelLoaded(537))Sleep(0);
  while(!pGame->IsModelLoaded(569))Sleep(0);
  while(!pGame->IsModelLoaded(449))Sleep(0);
  ScriptCommand(&r5CreateTrain,model,x,y,z,direction,&vehicleId);
  m_pVehicle=GamePool_Vehicle_GetAt(vehicleId);
  m_pEntity=(ENTITY_TYPE*)m_pVehicle;
  m_dwGTAId=vehicleId; r5LastCreatedVehicleId=vehicleId;
  r5LastCreatedVehicle=m_pVehicle;
  GamePrepareTrain(m_pVehicle);
 } else if(model==570 || model==569) {
  if(!r5LastCreatedVehicle) {
   m_pEntity=0; m_pVehicle=0; r5LastCreatedVehicle=0; return;
  }
  m_pVehicle=*(VEHICLE_TYPE**)((BYTE*)r5LastCreatedVehicle+0x5d4);
  if(!m_pVehicle) {
   pChatWindow->AddDebugMessage("Warning: bad train carriages");
   m_pEntity=0; m_pVehicle=0; r5LastCreatedVehicle=0; return;
  }
  vehicleId=GamePool_Vehicle_GetIndex(m_pVehicle);
  m_pEntity=(ENTITY_TYPE*)m_pVehicle;
  m_dwGTAId=vehicleId; r5LastCreatedVehicleId=vehicleId;
  r5LastCreatedVehicle=m_pVehicle;
 }
 VEH_DWORD(0x58)=FALSE; VEH_BYTE(0x64)=0; VEH_DWORD(0x65)=FALSE;
 VEH_DWORD(0xb4)=0; VEH_DWORD(0x6d)=FALSE;
 VEH_DWORD(0x69)=GetTickCount();
 VEH_DWORD(0x50)=0xffffffff; VEH_DWORD(0x54)=0xffffffff;
 VEH_DWORD(0x60)=FALSE; m_bHasNewColor=FALSE; field_71=0; VEH_DWORD(0x75)=0;
 VEH_DWORD(0x8f)=0;
 memset((BYTE*)this+0x93,0,33);
}
