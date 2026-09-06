#include "main.h"
#include "game/keystuff.h"
extern CNetGame *pNetGame;
void CMenuPool::Process()
{
	if (m_byteCurrentMenu == MAX_MENUS) return;

	GTA_CONTROLSET * pControls = GameGetInternalKeys();
	RakClientInterface* pRak = pNetGame->GetRakClient();
	if (pControls->wKeys1[16] && !pControls->wKeys2[16]) // Selected an item
	{
		BYTE row = m_pMenus[m_byteCurrentMenu]->GetSelectedRow();
		if (row != 0xFF)
		{
			m_byteExited = 1;
			RakNet::BitStream bsSend;
			bsSend.Write(row);
			pRak->RPC(RPC_MenuSelect, &bsSend, HIGH_PRIORITY, RELIABLE, 0, FALSE);
		}
	}
	else if (pControls->wKeys1[15] && !pControls->wKeys2[15]) // Exited
	{
		m_byteExited = 1;
		pRak->RPC(RPC_MenuQuit, NULL, HIGH_PRIORITY, RELIABLE, 0, FALSE);
	}
	else if (m_byteExited)
	{
		// Delay clearing for one frame to allow SA to make the nice noise
		HideMenu(m_byteCurrentMenu);
		m_byteExited = 0;
	}
}
