#include "main.h"
// C++ expansion of the three verified model helper bodies for Ob1.
DWORD CGame::CreateWeaponPickup(int iModel, DWORD dwAmmo, float fX, float fY, float fZ)
{
 DWORD hnd;
 if(iModel <= 20000 && iModel >= 0 && !ScriptCommand(&is_model_available,iModel)) {
  ScriptCommand(&request_model,iModel);
  ScriptCommand(&load_requested_models);
  while(iModel <= 20000 && iModel >= 0 && !ScriptCommand(&is_model_available,iModel)) Sleep(5);
 }
 ScriptCommand(&create_pickup_with_ammo,iModel,4,dwAmmo,fX,fY,fZ,&hnd);
 return hnd;
}
