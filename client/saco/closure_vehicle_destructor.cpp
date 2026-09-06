// Normal C++ lifetime with real IUnknown Release and source-derived native bridge.
#include "main.h"
#include "game/util.h"
extern CGame *pGame;
extern WORD __stdcall GetModelReferenceCount(int model);
extern const SCRIPT_COMMAND r5DestroyVehicle,r5DestroyTrain;
#define VEH_DWORD(offset) (*(DWORD*)((BYTE*)this+(offset)))
CVehicle::~CVehicle() {
 m_pVehicle=GamePool_Vehicle_GetAt(m_dwGTAId);
 if(m_pVehicle) {
  if(VEH_DWORD(0xb4)) {
   ScriptCommand(&disable_marker,VEH_DWORD(0xb4)); VEH_DWORD(0xb4)=0;
  }
  RemoveEveryoneFromVehicle();
  if(m_pTrailer) { DetachTrailer(); m_pTrailer=0; }
  IUnknown *&texture=*(IUnknown**)((BYTE*)this+0x8f);
  if(texture) { texture->Release(); texture=0; }
  if(m_pVehicle->entity.nModelIndex==538 || m_pVehicle->entity.nModelIndex==537) {
   ScriptCommand(&r5DestroyTrain,m_dwGTAId);
  } else {
   int model=m_pVehicle->entity.nModelIndex;
   ScriptCommand(&r5DestroyVehicle,m_dwGTAId);
   if(!GetModelReferenceCount(model) && !VEH_DWORD(0x87) &&
      pGame->GetLoadedVehicleModelCount()>80 && pGame->IsModelLoaded(model)) {
    // CStreaming::RemoveModel(int), native cdecl. Same existing source idiom.
    _asm push model
    _asm mov edx,0x4089a0
    _asm call edx
    _asm pop edx
   }
  }
 }
}
