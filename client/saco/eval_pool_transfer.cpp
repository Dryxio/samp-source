// Isolated 0.2.5 transfer evaluation; not accepted source coverage.
#include "main.h"
#define INVALID_OBJECT_ID 0xFFFF
class CObjectPoolTransfer {
 int count;
 BOOL m_bObjectSlotState[MAX_OBJECTS];
 CEntity *m_pObjects[MAX_OBJECTS];
 public:
 CEntity *GetObjFromGtaPtr(ENTITY_TYPE *pGtaObject);
 int FindIDFromGtaPtr(ENTITY_TYPE *pGtaObject);
};
class CPickupPoolTransfer {
 int m_iPickupCount;
 DWORD m_dwHnd[MAX_PICKUPS];
 int networkIds[MAX_PICKUPS];
 DWORD m_iTimer[MAX_PICKUPS];
 struct Dropped { bool bDroppedWeapon; WORD fromPlayer; } m_droppedWeapon[MAX_PICKUPS];
 char records[MAX_PICKUPS*20];
 public:
 void Destroy(int iPickup);
 void DestroyDropped(WORD fromPlayer);
};
CEntity* CObjectPoolTransfer::GetObjFromGtaPtr(ENTITY_TYPE * pGtaObject)
{
	int x=1;

	while(x <= count) {
		if (m_pObjects[x])
		{
			if(pGtaObject == m_pObjects[x]->m_pEntity) return m_pObjects[x];
		}
		x++;
	}

	return NULL;
}

int CObjectPoolTransfer::FindIDFromGtaPtr(ENTITY_TYPE * pGtaObject)
{
	int x=1;

	while(x <= count) {
		if (m_pObjects[x])
		{
			if(pGtaObject == m_pObjects[x]->m_pEntity) return x;
		}
		x++;
	}

	return INVALID_OBJECT_ID;
}

void CPickupPoolTransfer::Destroy(int iPickup)
{
	if (m_iPickupCount <= 0 || iPickup < 0 || iPickup >= MAX_PICKUPS) return;
	if (m_dwHnd[iPickup] != NULL)
	{
		ScriptCommand(&destroy_pickup, m_dwHnd[iPickup]);
		m_dwHnd[iPickup] = NULL;
		m_iTimer[iPickup] = NULL;
	networkIds[iPickup] = -1;
		m_iPickupCount--;
	}
}

void CPickupPoolTransfer::DestroyDropped(WORD fromPlayer)
{
	for (int i = 0; i < MAX_PICKUPS; i++)
	{
		if (m_dwHnd[i] != NULL && m_droppedWeapon[i].bDroppedWeapon && m_droppedWeapon[i].fromPlayer == fromPlayer)
		{
			ScriptCommand(&destroy_pickup, m_dwHnd[i]);
			m_dwHnd[i] = NULL;
			m_iTimer[i] = NULL;
			m_iPickupCount--;
		}
	}
}
