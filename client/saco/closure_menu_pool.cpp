// Generated complete definitions from net/menupool.cpp; see tools/prepare_actor_closure.py.
#include <time.h>
#include <math.h>
#include "main.h"
#include "game/util.h"
extern CHAR g_szMenuItems[MAX_MENU_ITEMS][MAX_COLUMNS][MAX_MENU_LINE+1];


CMenuPool::CMenuPool()
{
	// loop through and initialize all net players to null and slot states to false
	for (BYTE byteMenuID = 0; byteMenuID < MAX_MENUS; byteMenuID++)
	{
		m_bMenuSlotState[byteMenuID] = FALSE;
		m_pMenus[byteMenuID] = NULL;
	}
	m_byteCurrentMenu = MAX_MENUS;
}

CMenuPool::~CMenuPool()
{
	if (m_byteCurrentMenu != MAX_MENUS) m_pMenus[m_byteCurrentMenu]->Hide();
	for (BYTE byteMenuID = 0; byteMenuID < MAX_MENUS; byteMenuID++)
	{
		if (m_pMenus[byteMenuID])
		{
			delete m_pMenus[byteMenuID];
			m_pMenus[byteMenuID] = NULL;
		}
	}
}

BOOL CMenuPool::Delete(BYTE byteMenuID)
{
	if (m_bMenuSlotState[byteMenuID] == FALSE || !m_pMenus[byteMenuID])
	{
		return FALSE;
	}

	m_bMenuSlotState[byteMenuID] = FALSE;
	delete m_pMenus[byteMenuID];
	m_pMenus[byteMenuID] = NULL;

	return TRUE;
}

void CMenuPool::ShowMenu(BYTE byteMenuID)
{
	if (byteMenuID >= MAX_MENUS) return;
	if (m_bMenuSlotState[byteMenuID] == FALSE || !m_pMenus[byteMenuID]) return;

	if (m_byteCurrentMenu != MAX_MENUS) m_pMenus[m_byteCurrentMenu]->Hide();
	m_pMenus[byteMenuID]->Show();
	m_byteCurrentMenu = byteMenuID;

	m_byteExited = 0;
}

void CMenuPool::HideMenu(BYTE byteMenuID)
{
	if (byteMenuID >= MAX_MENUS || m_byteCurrentMenu == MAX_MENUS) return;
	if (m_bMenuSlotState[byteMenuID] == FALSE || !m_pMenus[byteMenuID]) return;
	m_pMenus[byteMenuID]->Hide();
	m_byteCurrentMenu = MAX_MENUS;
}

PCHAR CMenuPool::GetTextPointer(PCHAR szName)
{
	if (m_byteCurrentMenu == MAX_MENUS) return NULL;
	CMenu* pMenu = m_pMenus[m_byteCurrentMenu];
	if (!pMenu) return NULL;

	if (!strcmp(szName, "HED")) return pMenu->GetMenuTitle();
	if (!strcmp(szName, "RW1")) return pMenu->GetMenuHeader(0);
	if (!strcmp(szName, "RW2")) return pMenu->GetMenuHeader(1);
	for (BYTE i = 0; i < MAX_MENU_ITEMS; i++)
	{
		if (!strcmp(szName, g_szMenuItems[i][0] + 4)) return pMenu->GetMenuItem(0, i);
		if (!strcmp(szName, g_szMenuItems[i][1] + 4)) return pMenu->GetMenuItem(1, i);
	}
	return NULL;
}
