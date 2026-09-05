
#include "../main.h"

//----------------------------------------------------

CActorPool::CActorPool()
{
	for(ACTORID ActorID = 0; ActorID < MAX_ACTORS; ActorID++) {
		m_bSlotState[ActorID] = 0;
		m_pActors[ActorID] = 0;
		m_iGtaActorID[ActorID] = 0;
	}
	field_0 = 0;
}

//----------------------------------------------------

// Ordinary C++ reconstruction. Original full bodies are pinned in checkpoint32.
void CActorPool::UpdateCount() {
 int last = 0;
 for(int i=0;i<MAX_ACTORS;++i) if(m_bSlotState[i]) last=i;
 field_0=last;
}
BOOL CActorPool::Delete(ACTORID id) {
 if(id<MAX_ACTORS && m_bSlotState[id] && m_pActors[id]) {
  m_bSlotState[id]=FALSE;
  delete m_pActors[id];
  m_pActors[id]=NULL;
  m_iGtaActorID[id]=0;
  UpdateCount();
  return TRUE;
 }
 return FALSE;
}
ACTORID CActorPool::FindIDFromGtaPtr(int gtaPtr) {
 int i=0;
 if(field_0>=0) {
  do {if(gtaPtr==m_iGtaActorID[i]) return (ACTORID)i;++i;} while(i<=field_0);
 }
 return (ACTORID)0xFFFF;
}
void CActorPool::DeleteAll() {
 int i=0;
 do {Delete((ACTORID)i);++i;} while((ACTORID)i<MAX_ACTORS);
}

extern CChatWindow *pChatWindow;

BOOL CActorPool::New(ACTOR_SPAWN_INFO *info)
{
    if(m_pActors[info->id]) {
        pChatWindow->AddDebugMessage("Warning: actor %u was not deleted", info->id);
        Delete(info->id);
    }
    m_pActors[info->id] = new CActorPed(info->skin, info->x, info->y, info->z, info->rotation);
    if(m_pActors[info->id]) {
        m_iGtaActorID[info->id] = (int)m_pActors[info->id]->m_pPed;
        m_bSlotState[info->id] = TRUE;
        m_pActors[info->id]->SetHealth(info->health);
        if(info->immune) m_pActors[info->id]->ToggleImmunity(1);
        else m_pActors[info->id]->ToggleImmunity(0);
        field_2EE4[info->id] = TRUE;
        field_3E84[info->id] = 0;
        UpdateCount();
        return TRUE;
    }
    return FALSE;
}
