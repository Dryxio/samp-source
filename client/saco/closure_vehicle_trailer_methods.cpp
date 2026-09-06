#include "main.h"
#include "game/util.h"
extern BOOL r5TrailerDetachInProgress;
extern const SCRIPT_COMMAND r5PutTrailerOnCab;
extern const SCRIPT_COMMAND r5DetachTrailerFromCab;
void CVehicle::AttachTrailer()
{
 if(m_pTrailer)ScriptCommand(&r5PutTrailerOnCab,m_pTrailer->m_dwGTAId,m_dwGTAId);
}
void CVehicle::DetachTrailer()
{
 r5TrailerDetachInProgress=TRUE;
 if(m_pTrailer && m_pTrailer->m_dwGTAId && GamePool_Vehicle_GetAt(m_pTrailer->m_dwGTAId) && m_pTrailer->m_pVehicle)
  ScriptCommand(&r5DetachTrailerFromCab,m_pTrailer->m_dwGTAId,m_dwGTAId);
 r5TrailerDetachInProgress=FALSE;
}
void CVehicle::SetTrailer(CVehicle *pTrailer)
{
 m_pTrailer=pTrailer;
}
