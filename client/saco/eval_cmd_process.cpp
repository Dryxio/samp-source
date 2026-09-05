// Isolated 0.2.5 command-processing transfer using verified R5 layout.
#include "main.h"
extern CNetGame *pNetGame;
extern CChatWindow *pChatWindow;
class CChatProcessEval { public: void ResetPage(); };
#define CMD_CHARACTER '/'

#pragma once

#define MAX_CMD_INPUT   128
#define MAX_CMDS		144
#define MAX_CMD_STRLEN  32

typedef void (__cdecl *CMDPROC)(PCHAR);

//----------------------------------------------------

class CCmdProcessEval
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
	char m_szRecallBuffer[10][MAX_CMD_INPUT+1];
	char m_szCurBuffer[MAX_CMD_INPUT+1];
	int m_iCurrentRecallAt;
	int m_iTotalRecalls;
	CMDPROC		m_pDefaultCmd;	 // used when no command specifier was
								 // used (ie. a normal chat message)

	void GetDialogSize(RECT *pRect);

	void Enable();
	void Disable();

	void ProcessInput();
 void SendToServer(char *szServerCommand);
 int MsgProc(UINT uMsg, DWORD wParam, DWORD lParam);
	void AddToRecallBuffer(char *szCmdInput);
	void RecallUp();
	void RecallDown();

	CMDPROC GetCmdHandler(PCHAR szCmdName);
	void AddDefaultCmdProc(CMDPROC cmdDefault);
	void AddCmdProc(PCHAR szCmdName, CMDPROC cmdHandler);

	void ResetDialogControls(CDXUTDialog *pGameUI);
	CCmdProcessEval(IDirect3DDevice9 *pD3DDevice);
	~CCmdProcessEval();
};

//----------------------------------------------------
// EOF

void CCmdProcessEval::ProcessInput()
{
	PCHAR szCmdEndPos;
	CMDPROC cmdHandler;

	if(!m_pEditControl) return;

	strncpy(m_szInputBuffer,m_pEditControl->GetTextA(),MAX_CMD_INPUT);
	m_szInputBuffer[MAX_CMD_INPUT] = '\0';
    
	// don't process 0 length input
	int inputLength;
	if(!(inputLength = strlen(m_szInputBuffer))) {
		if(m_bEnabled) {
			if(pChatWindow) ((CChatProcessEval*)pChatWindow)->ResetPage();
			Disable();
		}
		return;
	}

    // remember this command for later use in the recalls.	
    AddToRecallBuffer(m_szInputBuffer);
	m_iCurrentRecallAt = -1;

	if(*m_szInputBuffer != CMD_CHARACTER) { 
		// chat type message	
		if(m_pDefaultCmd) {
			m_pDefaultCmd(m_szInputBuffer);
		}
	}
	else 
	{// possible valid command
		// find the end of the name
		szCmdEndPos = m_szInputBuffer + 1;
		while(*szCmdEndPos && *szCmdEndPos != ' ') szCmdEndPos++;
		if(*szCmdEndPos == '\0') {
			// Possible command with no params.
			cmdHandler = GetCmdHandler(m_szInputBuffer + 1);

			// If valid then call it.
			if(cmdHandler) {
				cmdHandler("");
			}
			else {
				if (pNetGame) {
					SendToServer(m_szInputBuffer);
				}
				else {
					pChatWindow->AddDebugMessage("I don't know that command.");
				}
			}
		}
		else {
			char szCopiedBuffer[MAX_CMD_INPUT+1];
			strcpy(szCopiedBuffer, m_szInputBuffer);

			*szCmdEndPos='\0'; // null terminate it
			szCmdEndPos++; // rest is the parameters.
			cmdHandler = GetCmdHandler(m_szInputBuffer + 1);

			// If valid then call it with the param string.
			if(cmdHandler) {
				cmdHandler(szCmdEndPos);
			}
			else {
				if (pNetGame) {
					SendToServer(szCopiedBuffer);
				}


				else {
					pChatWindow->AddDebugMessage("I don't know that command.");
				}
			}
		}
	}

	*m_szInputBuffer ='\0';	
	m_pEditControl->SetText("",false);

	if(m_bEnabled) {
		if(pChatWindow) ((CChatProcessEval*)pChatWindow)->ResetPage();
		Disable();
	}
}
void CCmdProcessEval::SendToServer(char* szServerCommand)
{
	if(!pNetGame) return;

	RakNet::BitStream bsParams;
	int iStrlen = strlen(szServerCommand);

	//pChatWindow->AddDebugMessage("SendToServer(%s,%u)",szServerCommand,iStrlen);

	bsParams.Write(iStrlen);
	bsParams.Write(szServerCommand, iStrlen);
	pNetGame->GetRakClient()->RPC(RPC_ServerCommand, &bsParams, HIGH_PRIORITY, RELIABLE, 0, FALSE);

}
int CCmdProcessEval::MsgProc(UINT uMsg, DWORD wParam, DWORD lParam)
{
	if(m_bEnabled && m_pEditControl) {
		if(CDXUTIMEEditBox::StaticMsgProc(uMsg,wParam,lParam)) return 1;
	}
	return 0;
}
