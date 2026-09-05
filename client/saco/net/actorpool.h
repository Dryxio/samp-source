
#pragma once

//----------------------------------------------------

// Packed R5 actor creation payload, 27 bytes.
struct ACTOR_SPAWN_INFO {
    ACTORID id;
    int skin;
    float x, y, z;
    float rotation;
    float health;
    BYTE immune;
};

class CActorPool
{
private:
	int field_0;
	CActorPed* m_pActors[MAX_ACTORS];
	BOOL m_bSlotState[MAX_ACTORS];
	int m_iGtaActorID[MAX_ACTORS];

	BOOL field_2EE4[MAX_ACTORS];
	int field_3E84[MAX_ACTORS];

public:

	CActorPool();
	void UpdateCount();
	BOOL Delete(ACTORID id);
	ACTORID FindIDFromGtaPtr(int gtaPtr);
	void DeleteAll();
	BOOL New(ACTOR_SPAWN_INFO *info);
};

//----------------------------------------------------
