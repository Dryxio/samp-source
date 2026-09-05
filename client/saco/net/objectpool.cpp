
#include "../main.h"

CObjectPool::CObjectPool()
{
	for(WORD wObjectID = 0; wObjectID < MAX_OBJECTS; wObjectID++)
	{
		m_bObjectSlotState[wObjectID] = 0;
		m_pObjects[wObjectID] = 0;
	}
	m_iLastObjectId = 0;
};

// Transferred from michael-fa0.2.5; R5 uses its inclusive last-object ID
// and invalid ID65535. CObject remains incomplete; only its verified Entity
// base field at0x40 is accessed. No CObject implementation is counted.

CObject* CObjectPool::GetObjFromGtaPtr(ENTITY_TYPE * pGtaObject)
{
	int x=1;

	while(x <= m_iLastObjectId) {
		if (m_pObjects[x])
		{
			if(pGtaObject == reinterpret_cast<CEntity *>(m_pObjects[x])->m_pEntity) return m_pObjects[x];
		}
		x++;
	}

	return NULL;
}

int CObjectPool::FindIDFromGtaPtr(ENTITY_TYPE * pGtaObject)
{
	int x=1;

	while(x <= m_iLastObjectId) {
		if (m_pObjects[x])
		{
			if(pGtaObject == reinterpret_cast<CEntity *>(m_pObjects[x])->m_pEntity) return x;
		}
		x++;
	}

	return 0xFFFF;
}

typedef char complete_object_pool_size[(sizeof(CObjectPool)==0x1F44)?1:-1];
