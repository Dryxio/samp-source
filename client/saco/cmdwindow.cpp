
#include "main.h"
extern CNetGame *pNetGame;
extern CChatWindow *pChatWindow;
#define CMD_CHARACTER '/'

extern CGame		*pGame;
extern CConfig		*pConfig;

//----------------------------------------------------

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

//----------------------------------------------------

CCmdWindow::~CCmdWindow()
{
}

//----------------------------------------------------

void CCmdWindow::ResetDialogControls(CDXUTDialog *pGameUI)
{
	m_pGameUI = pGameUI;

	if(pGameUI) {
		pGameUI->AddIMEEditBox(IDC_CMDEDIT,"",10,175,570,40,true,&m_pEditControl);
		if(pConfig->GetIntVariable("ime")) {
			CDXUTIMEEditBox::EnableImeSystem(true);
			CDXUTIMEEditBox::StaticOnCreateDevice();
		}
		m_pEditControl->GetElement(0)->TextureColor.Init(D3DCOLOR_ARGB( 240, 5, 5, 5 ));
		m_pEditControl->SetTextColor(D3DCOLOR_ARGB( 255, 255, 255, 255 ));
		m_pEditControl->SetCaretColor(D3DCOLOR_ARGB( 255, 150, 150, 150 ));
		m_pEditControl->SetSelectedBackColor(D3DCOLOR_ARGB( 255, 185, 34, 40 ));
		m_pEditControl->SetSelectedTextColor(D3DCOLOR_ARGB( 255, 10, 10, 15 ));
		m_pEditControl->SetEnabled(false);
		m_pEditControl->SetVisible(false);
	}
}

//----------------------------------------------------

void CCmdWindow::GetDialogSize(RECT *pRect)
{
	memset(pRect,0,sizeof(RECT));

	if(m_pGameUI) {
		pRect->right = m_pGameUI->GetWidth();
		pRect->bottom = m_pGameUI->GetHeight();
	}
}

//----------------------------------------------------

void CCmdWindow::Enable()
{
    if(m_bEnabled) return;
    if(m_pEditControl) {
        RECT rect;
        GetClientRect(pGame->GetMainWindowHwnd(), &rect);
        int width = (int)(rect.right * 0.6f);
        if(width > 800) width = 800;
        m_pEditControl->SetEnabled(true);
        m_pEditControl->SetVisible(true);
        m_pEditControl->SetLocation(40, pChatWindow->GetInputLineY());
        m_pEditControl->SetSize(width, 14 - (int)(m_pGameUI->GetFont(0)->nHeight * -1.5f));
        m_pGameUI->RequestFocus(m_pEditControl);
        m_pEditControl->OnFocusIn();
        m_pGameUI->SetSize(width + 100, pChatWindow->GetInputLineY() + 50);
    }
    m_bEnabled = TRUE;
}

//----------------------------------------------------

void CCmdWindow::Disable()
{
	if(!m_bEnabled) return;

	if(m_pEditControl) {
		m_pEditControl->OnFocusOut();
		m_pEditControl->SetEnabled(false);
		m_pEditControl->SetVisible(false);
	}
	pGame->ToggleKeyInputsDisabled(0, TRUE);
	m_bEnabled = FALSE;
}

//----------------------------------------------------





//----------------------------------------------------

void CCmdWindow::ProcessInput()
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
			if(pChatWindow) pChatWindow->ResetPage();
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
		if(pChatWindow) pChatWindow->ResetPage();
		Disable();
	}
}

//----------------------------------------------------

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

//----------------------------------------------------

void CCmdWindow::AddDefaultCmdProc(CMDPROC cmdDefault)
{
	m_pDefaultCmd = cmdDefault;
}

//----------------------------------------------------

void CCmdWindow::AddCmdProc(PCHAR szCmdName, CMDPROC cmdHandler)
{
	if(m_iCmdCount < MAX_CMDS && (strlen(szCmdName) < MAX_CMD_STRLEN)) {
		m_pCmds[m_iCmdCount] = cmdHandler;
		strcpy(m_szCmdNames[m_iCmdCount],szCmdName);
		m_iCmdCount++;
	}
}

//----------------------------------------------------











// Recall methods transferred from0.2.5 with existing R5 layout and GetTextA.
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

typedef char complete_command_window_size[(sizeof(CCmdWindow)==0x1AFC)?1:-1];

void CCmdWindow::SendToServer(char* szServerCommand)
{
	if(!pNetGame) return;

	RakNet::BitStream bsParams;
	int iStrlen = strlen(szServerCommand);

	//pChatWindow->AddDebugMessage("SendToServer(%s,%u)",szServerCommand,iStrlen);

	bsParams.Write(iStrlen);
	bsParams.Write(szServerCommand, iStrlen);
	pNetGame->GetRakClient()->RPC(RPC_ServerCommand, &bsParams, HIGH_PRIORITY, RELIABLE, 0, FALSE);

}

int CCmdWindow::MsgProc(UINT uMsg, DWORD wParam, DWORD lParam)
{
	if(m_bEnabled && m_pEditControl) {
		if(CDXUTIMEEditBox::StaticMsgProc(uMsg,wParam,lParam)) return 1;
	}
	return 0;
}
