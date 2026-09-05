
#pragma once

class CObject;

class CObjectPool
{
private:
	int m_iLastObjectId;
	BOOL m_bObjectSlotState[MAX_OBJECTS];
	CObject *m_pObjects[MAX_OBJECTS];
public:
	CObjectPool();
	CObject *GetObjFromGtaPtr(ENTITY_TYPE *pGtaObject);
	int FindIDFromGtaPtr(ENTITY_TYPE *pGtaObject);

};
