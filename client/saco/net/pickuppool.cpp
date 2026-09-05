
#include "../main.h"

//----------------------------------------------------

CPickupPool::~CPickupPool()
{
	for(int i = 0; i < MAX_PICKUPS; i++)
	{
		if (m_dwHnd[i] != NULL)
		{
			ScriptCommand(&destroy_pickup, m_dwHnd[i]);
		}
	}
}

//----------------------------------------------------


// Transferred from michael-fa0.2.5 using the verified R5 layout and4096
// slots. Destroy additionally resets the R5 network-ID slot to-1.

void CPickupPool::Destroy(int iPickup)
{
	if (m_iPickupCount <= 0 || iPickup < 0 || iPickup >= MAX_PICKUPS) return;
	if (m_dwHnd[iPickup] != NULL)
	{
		ScriptCommand(&destroy_pickup, m_dwHnd[iPickup]);
		m_dwHnd[iPickup] = NULL;
		m_iTimer[iPickup] = NULL;
		m_iNetworkIds[iPickup] = -1;
		m_iPickupCount--;
	}
}

void CPickupPool::DestroyDropped(WORD fromPlayer)
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

typedef char complete_pickup_pool_size[(sizeof(CPickupPool)==0x23004)?1:-1];
