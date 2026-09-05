// Generated complete definitions from cmdwindow.cpp; see tools/prepare_actor_closure.py.
#include <time.h>
#include <math.h>
#include "main.h"
#include "game/util.h"


CCmdWindow::CCmdWindow(IDirect3DDevice9 *pD3DDevice)
{
	m_bEnabled				= FALSE;
	m_pD3DDevice			= pD3DDevice;
	m_iCmdCount				= 0;
	m_iTotalRecalls				= 0;
	m_pEditControl			= NULL;
	m_iCurrentRecallAt				= -1;

	memset(&m_szRecallBuffer[0],0,1290);
	memset(&m_szInputBuffer[0],0,(MAX_CMD_INPUT+1));
	memset(&m_szCurBuffer[0],0,129);
}

void CCmdWindow::AddToRecallBuffer(char *szCmdInput)
{
	// Move all the existing recalls up 1
    int x=10-1;
	while(x) {
		strcpy(m_szRecallBuffer[x],m_szRecallBuffer[x-1]);
		x--;
	}
	// Copy this into the first recall slot
    strcpy(m_szRecallBuffer[0],szCmdInput);
	if(m_iTotalRecalls < 10) {
		m_iTotalRecalls++;
	}
}

void CCmdWindow::RecallUp()
{
	if(m_iCurrentRecallAt >= (m_iTotalRecalls - 1)) return;

	if(m_iCurrentRecallAt == -1) {
		// Save the current buffer incase we want to return to it.
		strncpy(m_szCurBuffer,m_pEditControl->GetTextA(),MAX_CMD_INPUT);
		m_szCurBuffer[MAX_CMD_INPUT] = '\0';
	}

	m_iCurrentRecallAt++;
	m_pEditControl->SetText(m_szRecallBuffer[m_iCurrentRecallAt]);
	//pChatWindow->AddDebugMessage("RecallAt: %d",m_iCurrentRecallAt);	
}

void CCmdWindow::RecallDown()
{
	m_iCurrentRecallAt--;
	if(m_iCurrentRecallAt >= 0) {
		m_pEditControl->SetText(m_szRecallBuffer[m_iCurrentRecallAt]);
		//pChatWindow->AddDebugMessage("RecallAt: %d",m_iCurrentRecallAt);
	} else {
		if(m_iCurrentRecallAt == -1) {
			m_pEditControl->SetText(m_szCurBuffer);
			//pChatWindow->AddDebugMessage("RecallAt: -cur-");
		}
		m_iCurrentRecallAt = -1;		
	}	
}

void CCmdWindow::GetDialogSize(RECT *pRect)
{
	memset(pRect,0,sizeof(RECT));

	if(m_pGameUI) {
		pRect->right = m_pGameUI->GetWidth();
		pRect->bottom = m_pGameUI->GetHeight();
	}
}

CMDPROC CCmdWindow::GetCmdHandler(PCHAR szCmdName)
{
	int x=0;
	while(x!=m_iCmdCount) {
		if(!stricmp(szCmdName,m_szCmdNames[x])) {
			return m_pCmds[x];
		}
		x++;
	}
	return NULL;
}

void CCmdWindow::AddDefaultCmdProc(CMDPROC cmdDefault)
{
	m_pDefaultCmd = cmdDefault;
}

void CCmdWindow::AddCmdProc(PCHAR szCmdName, CMDPROC cmdHandler)
{
	if(m_iCmdCount < MAX_CMDS && (strlen(szCmdName) < MAX_CMD_STRLEN)) {
		m_pCmds[m_iCmdCount] = cmdHandler;
		strcpy(m_szCmdNames[m_iCmdCount],szCmdName);
		m_iCmdCount++;
	}
}
