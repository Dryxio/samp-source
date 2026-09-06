// 0.2.5 InitMenu transfer with R5 buffering, bounds and title application.
#include "main.h"
extern CNetGame *pNetGame;
extern char rpc_menu_text_buffer[257];
struct RPCInitMenuNetGameView { BYTE gap[0x3de]; NETGAME_POOLS *pools; };
struct RPCInitMenuPoolView {
 CMenu *menus[128]; BOOL slots[128];
 BOOL GetSlotState(BYTE id) { if(id>128)return FALSE; return slots[id]; }
};
void ScrInitMenuTransfer(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;
	PlayerID sender = rpcParams->sender;

	RakNet::BitStream bsData(Data,(iBitLength/8)+1,false);
	
	if(!pNetGame) return;
	CMenuPool* pMenuPool = ((RPCInitMenuNetGameView*)pNetGame)->pools->pMenuPool;

	BYTE byteMenuID;
	BOOL bColumns; // 0 = 1, 1 = 2
	
	float fX;
	float fY;
	float fCol1;
	float fCol2 = 0.0;
	MENU_INT MenuInteraction;
	
	bsData.Read(byteMenuID);
	bsData.Read(bColumns);
	memset(rpc_menu_text_buffer,0,sizeof(rpc_menu_text_buffer));
	bsData.Read(rpc_menu_text_buffer, MAX_MENU_LINE);
	bsData.Read(fX);
	bsData.Read(fY);
	bsData.Read(fCol1);
	if (bColumns) bsData.Read(fCol2);
	bsData.Read(MenuInteraction.bMenu);
	for (BYTE i = 0; i < MAX_MENU_ITEMS; i++)
	{
		bsData.Read(MenuInteraction.bRow[i]);
	}

	CMenu* pMenu;
	
	if (((RPCInitMenuPoolView*)pMenuPool)->GetSlotState(byteMenuID))
	{
		pMenuPool->Delete(byteMenuID);
	}
	
	pMenu = pMenuPool->New(byteMenuID, fX, fY, ((BYTE)bColumns) + 1, fCol1, fCol2, &MenuInteraction);
	
	if (!pMenu) return;
	pMenu->SetTitle(rpc_menu_text_buffer);
	
	BYTE byteColCount;
	memset(rpc_menu_text_buffer,0,sizeof(rpc_menu_text_buffer));
	bsData.Read(rpc_menu_text_buffer, MAX_MENU_LINE);
	pMenu->SetColumnTitle(0, rpc_menu_text_buffer);
	
	bsData.Read(byteColCount);
	if(byteColCount>MAX_MENU_ITEMS)return;
	for (BYTE i = 0; i < byteColCount; i++)
	{
		memset(rpc_menu_text_buffer,0,sizeof(rpc_menu_text_buffer));
	bsData.Read(rpc_menu_text_buffer, MAX_MENU_LINE);
		pMenu->AddMenuItem(0, i, rpc_menu_text_buffer);
	}
	
	if (bColumns)
	{
		memset(rpc_menu_text_buffer,0,sizeof(rpc_menu_text_buffer));
	bsData.Read(rpc_menu_text_buffer, MAX_MENU_LINE);
		pMenu->SetColumnTitle(1, rpc_menu_text_buffer);
		
		bsData.Read(byteColCount);
	if(byteColCount>MAX_MENU_ITEMS)return;
		for (BYTE i = 0; i < byteColCount; i++)
		{
			memset(rpc_menu_text_buffer,0,sizeof(rpc_menu_text_buffer));
	bsData.Read(rpc_menu_text_buffer, MAX_MENU_LINE);
			pMenu->AddMenuItem(1, i, rpc_menu_text_buffer);
		}
	}
}

CMenu* CMenuPool::New(BYTE byteMenuID, float fX, float fY, BYTE byteColumns, float fCol1Width, float fCol2Width, MENU_INT *MenuInteraction)
{
	SAFE_DELETE(m_pMenus[byteMenuID]);
	m_bMenuSlotState[byteMenuID] = FALSE;
	CMenu* pMenu = new CMenu(fX, fY, byteColumns, fCol1Width, fCol2Width, MenuInteraction);

	if (pMenu)
	{
		m_bMenuSlotState[byteMenuID] = TRUE;
		m_pMenus[byteMenuID] = pMenu;
		return pMenu;
	}
	return NULL;
}

