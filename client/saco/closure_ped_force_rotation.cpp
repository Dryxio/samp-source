// Source0.2.5 ForceTargetRotation, adapted R5 rotation field write order.
#include "main.h"
#include "game/util.h"
void CPlayerPed::ForceTargetRotation(float rotation) {
 if(!m_pPed)return;
 if(!GamePool_Ped_GetAt(m_dwGTAId))return;
 m_pPed->fRotation2=DegToRad(rotation);
 m_pPed->fRotation1=DegToRad(rotation);
 ScriptCommand(&set_actor_z_angle,m_dwGTAId,rotation);
}
