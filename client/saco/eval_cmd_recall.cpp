// Isolated0.2.5 recall transfer using existing R5 class offsets; unaccepted.
#include "main.h"
#define MAX_RECALLS 10

#pragma once

#define MAX_CMD_INPUT   128
#define MAX_CMDS		144
#define MAX_CMD_STRLEN  32

typedef void (__cdecl *CMDPROC)(PCHAR);

//----------------------------------------------------

class CCmdRecallEval
{
private:

	IDirect3DDevice9 *m_pD3DDevice;
	CDXUTDialog		*m_pGameUI;
	CDXUTIMEEditBox	*m_pEditControl;

public:

	CMDPROC		m_pCmds[MAX_CMDS];
	CHAR        m_szCmdNames[MAX_CMDS][MAX_CMD_STRLEN+1];
	int			m_iCmdCount;

	BOOL		m_bEnabled;

	char		m_szInputBuffer[MAX_CMD_INPUT + 1];
	char m_szRecallBuffer[10][129];
	char m_szCurBuffer[129];
	int m_iCurrentRecallAt;
	int m_iTotalRecalls;
	CMDPROC		m_pDefaultCmd;	 // used when no command specifier was
								 // used (ie. a normal chat message)

	void GetDialogSize(RECT *pRect);

	void Enable();
	void Disable();

	void ProcessInput();
 void AddToRecallBuffer(char *szCmdInput);
 void RecallUp();
 void RecallDown();

	CMDPROC GetCmdHandler(PCHAR szCmdName);
	void AddDefaultCmdProc(CMDPROC cmdDefault);
	void AddCmdProc(PCHAR szCmdName, CMDPROC cmdHandler);

	void ResetDialogControls(CDXUTDialog *pGameUI);
	CCmdRecallEval(IDirect3DDevice9 *pD3DDevice);
	~CCmdRecallEval();
};

//----------------------------------------------------
// EOF

void CCmdRecallEval::AddToRecallBuffer(char *szCmdInput)
{
	// Move all the existing recalls up 1
    int x=MAX_RECALLS-1;
	while(x) {
		strcpy(m_szRecallBuffer[x],m_szRecallBuffer[x-1]);
		x--;
	}
	// Copy this into the first recall slot
    strcpy(m_szRecallBuffer[0],szCmdInput);
	if(m_iTotalRecalls < MAX_RECALLS) {
		m_iTotalRecalls++;
	}
}
void CCmdRecallEval::RecallUp()
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
void CCmdRecallEval::RecallDown()
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
