
#include "main.h"

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

		m_pEditControl->SetEnabled(true);
		m_pEditControl->SetVisible(true);

		// TODO: CCmdWindow::Enable()
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
	if(!m_pEditControl) return;

	strncpy(m_szInputBuffer,m_pEditControl->GetTextA(),MAX_CMD_INPUT);
	m_szInputBuffer[MAX_CMD_INPUT] = '\0';

	// TODO: CCmdWindow::ProcessInput()
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
