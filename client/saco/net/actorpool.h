
#pragma once

//----------------------------------------------------

class CActorPool
{
private:
	int field_0;
	CActorPed* m_pActors[MAX_ACTORS];
	BOOL m_bSlotState[MAX_ACTORS];
	int m_iGtaActorID[MAX_ACTORS];

	char _gap2EE4[8000];

public:

	CActorPool();
	void UpdateCount();
	BOOL Delete(ACTORID id);
	ACTORID FindIDFromGtaPtr(int gtaPtr);
	void DeleteAll();
};

//----------------------------------------------------
