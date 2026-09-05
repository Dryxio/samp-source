// Isolated recovery of R5 command opening. Layouts mirror the accepted classes.
#include "main.h"
extern CGame *pGame;
extern CChatWindow *pChatWindow;

#pragma once

#define MAX_CMD_INPUT   128
#define MAX_CMDS		144
#define MAX_CMD_STRLEN  32

typedef void (__cdecl *CMDPROC)(PCHAR);

//----------------------------------------------------

class CCmdEnableEval
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
	void AddToRecallBuffer(char *szCmdInput);
	void RecallUp();
	void RecallDown();

	CMDPROC GetCmdHandler(PCHAR szCmdName);
	void AddDefaultCmdProc(CMDPROC cmdDefault);
	void AddCmdProc(PCHAR szCmdName, CMDPROC cmdHandler);

	void ResetDialogControls(CDXUTDialog *pGameUI);
	CCmdEnableEval(IDirect3DDevice9 *pD3DDevice);
	~CCmdEnableEval();
};

//----------------------------------------------------
// EOF


#pragma once

#define MAX_MESSAGES				100


#pragma pack(1)
typedef struct _CHAT_ENABLE_ENTRY
{
	DWORD timestamp;
	char prefix[28];
	char text[208];
	int type;
	DWORD textColor;
	DWORD prefixColor;
} CHAT_ENABLE_ENTRY;

class CChatEnableEval // size: 25578
{
private:

	int field_0;
	char _gap4[4];
	int field_8;
	char field_C;
	int field_D;
	char field_11[261];
	CDXUTDialog			*m_pGameUI;
	int field_11A;
	CDXUTScrollBar		*m_pScrollBar;
	DWORD				m_dwChatTextColor;
	DWORD				m_dwChatInfoColor;
	DWORD				m_dwChatDebugColor;
	int m_iInputLineY;
	CHAT_ENABLE_ENTRY	m_ChatWindowEntries[MAX_MESSAGES];
	CFontRender			*m_pFontRender;
	ID3DXSprite			*field_63A6;
	ID3DXSprite			*field_63AA;
	IDirect3DDevice9	*m_pD3DDevice;
	int field_63B2;
	ID3DXRenderToSurface *field_63B6;
	IDirect3DTexture9* field_63BA;
	int field_63BE;
	D3DDISPLAYMODE field_63C2;
	DWORD field_63D2;
	int field_63D6;
	int field_63DA;
	int field_63DE;
	int field_63E2;
	char _gap63E6[4];


	void CreateFonts();

	void FUNC_10067200();

public:

	void AddDebugMessage(CHAR *szFormat, ...);
	void AddEntry(int type, CHAR *text, CHAR *prefix, DWORD textColor, DWORD prefixColor);
	void Log(int type, CHAR *text, CHAR *prefix);
	void PushBack();
	void ResetPage();
 int GetInputLineY() const { return m_iInputLineY; }

	void ResetDialogControls(CDXUTDialog *pGameUI);

	CChatEnableEval(IDirect3DDevice9 *pD3DDevice, CFontRender *pFontRender, CHAR *szChatLogFile);
};

void CCmdEnableEval::Enable()
{
    if(m_bEnabled) return;
    if(m_pEditControl) {
        RECT rect;
        GetClientRect(pGame->GetMainWindowHwnd(), &rect);
        int width = (int)(rect.right * 0.6f);
        if(width > 800) width = 800;
        m_pEditControl->SetEnabled(true);
        m_pEditControl->SetVisible(true);
        m_pEditControl->SetLocation(40, ((CChatEnableEval*)pChatWindow)->GetInputLineY());
        m_pEditControl->SetSize(width, 14 - (int)(m_pGameUI->GetFont(0)->nHeight * -1.5f));
        m_pGameUI->RequestFocus(m_pEditControl);
        m_pEditControl->OnFocusIn();
        m_pGameUI->SetSize(width + 100, ((CChatEnableEval*)pChatWindow)->GetInputLineY() + 50);
    }
    m_bEnabled = TRUE;
}
