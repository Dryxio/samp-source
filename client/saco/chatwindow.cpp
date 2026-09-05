#include <time.h>

#include "main.h"

//----------------------------------------------------

CChatWindow::CChatWindow(IDirect3DDevice9 *pD3DDevice, CFontRender *pFontRender, CHAR *szChatLogFile)
{
	int x=0;

	m_pD3DDevice		= pD3DDevice;
	m_pFontRender		= pFontRender;
	field_8 = 2;

	// Create a sprite to use when drawing text
	D3DXCreateSprite(pD3DDevice,&field_63A6);
	D3DXCreateSprite(pD3DDevice,&field_63AA);

	// Init the chat window lines to 0
	while(x!=MAX_MESSAGES) {
		memset(&m_ChatWindowEntries[x],0,sizeof(CHAT_WINDOW_ENTRY));
		x++;
	}

	m_dwChatTextColor = D3DCOLOR_ARGB(255,255,255,255);
	m_dwChatInfoColor = D3DCOLOR_ARGB(255,136,170,98);
	m_dwChatDebugColor = D3DCOLOR_ARGB(255,169,196,228);

	field_0 = 10;
	field_C = 0;

	if(szChatLogFile && strlen(szChatLogFile))
	{
		memset(&field_11[0],0,sizeof(field_11));
		strncpy(field_11,szChatLogFile,MAX_PATH);

		FILE *f = fopen(field_11, "w");
		if(f)
		{
			field_D = 1;
			fclose(f);
		}
	}

	field_63BE = 0;
	field_63BA = NULL;
	field_63B6 = NULL;
	field_63D2 = GetTickCount();
	field_63DE = 1;
	m_pScrollBar = NULL;
	m_pGameUI = NULL;
	field_11A = 0;

	CreateFonts();
}

//----------------------------------------------------

void CChatWindow::CreateFonts()
{

	// TODO: CChatWindow::CreateFonts .text:100681D0
}

//----------------------------------------------------
// MATCH
void CChatWindow::ResetDialogControls(CDXUTDialog *pGameUI)
{
	m_pGameUI = pGameUI;

	if(pGameUI) {
		m_pScrollBar = new CDXUTScrollBar(pGameUI);
		pGameUI->AddControl(m_pScrollBar);
		m_pScrollBar->SetVisible(true);
		m_pScrollBar->SetEnabled(true);

		FUNC_10067200();
	}
}

//----------------------------------------------------
//----------------------------------------------------

void CChatWindow::AddDebugMessage(CHAR * szFormat, ...)
{
    // Complete C++ reconstruction of the R5 formatting and sanitizing path.
    char buffer[512];
    memset(buffer, 0, sizeof(buffer));
    va_list args;
    va_start(args, szFormat);
    vsprintf(buffer, szFormat, args);
    va_end(args);
    for(char *p = buffer; *p; ++p) {
        if(*p > 0 && *p < 32) *p = ' ';
    }
    AddEntry(8, buffer, NULL, m_dwChatDebugColor, 0);
}
// MATCH
void CChatWindow::FUNC_10067200()
{
	if(m_pScrollBar) {
		m_pScrollBar->SetLocation(10,40);
		m_pScrollBar->SetSize(20,((field_63E2+1)*field_0)-60);
		m_pScrollBar->SetTrackRange(1,MAX_MESSAGES);
		m_pScrollBar->SetPageSize(field_0);
		m_pScrollBar->SetTrackPos(MAX_MESSAGES-field_0);
	}
}


inline void CChatWindow::PushBack()
{
    memcpy(m_ChatWindowEntries, m_ChatWindowEntries + 1, sizeof(m_ChatWindowEntries));
}

// R5 copies all 100 entries from the next slot, including its original over-read.
// The prefix guard deliberately follows that copy.
void CChatWindow::AddEntry(int type, CHAR *text, CHAR *prefix, DWORD textColor, DWORD prefixColor)
{
    PushBack();
    if(prefix && strlen(prefix) > 24) return;
    m_ChatWindowEntries[99].type = type;
    m_ChatWindowEntries[99].textColor = textColor;
    m_ChatWindowEntries[99].prefixColor = prefixColor;
    m_ChatWindowEntries[99].timestamp = (DWORD)time(NULL);
    if(prefix) {
        strcpy(m_ChatWindowEntries[99].prefix, prefix);
        strcat(m_ChatWindowEntries[99].prefix, ":");
    } else m_ChatWindowEntries[99].prefix[0] = 0;
    Log(type, text, prefix);
    if(m_ChatWindowEntries[99].type == 2 && strlen(text) > 72) {
        int split = 72;
        while(text[split] != ' ' && split) --split;
        if(72 - split > 12) {
            strncpy(m_ChatWindowEntries[99].text, text, 72);
            m_ChatWindowEntries[99].text[72] = 0;
            PushBack();
            m_ChatWindowEntries[99].type = type;
            m_ChatWindowEntries[99].textColor = textColor;
            m_ChatWindowEntries[99].prefixColor = prefixColor;
            m_ChatWindowEntries[99].prefix[0] = 0;
            strcpy(m_ChatWindowEntries[99].text, text + 72);
        } else {
            strncpy(m_ChatWindowEntries[99].text, text, split);
            m_ChatWindowEntries[99].text[split] = 0;
            PushBack();
            m_ChatWindowEntries[99].type = type;
            m_ChatWindowEntries[99].textColor = textColor;
            m_ChatWindowEntries[99].prefixColor = prefixColor;
            m_ChatWindowEntries[99].prefix[0] = 0;
            strcpy(m_ChatWindowEntries[99].text, text + split + 1);
        }
    } else {
        strncpy(m_ChatWindowEntries[99].text, text, 144);
        m_ChatWindowEntries[99].text[144] = 0;
    }
    field_63DA = 1;
}

void CChatWindow::Log(int type, CHAR *text, CHAR *prefix)
{
    time_t now = time(NULL);
    if(!field_D) return;
    FILE *file = fopen(field_11, "a");
    if(!file) return;
    char stamp[65];
    strftime(stamp, 64, "[%H:%M:%S]", localtime(&now));
    switch(type) {
        case 2:
            fprintf(file, "%s <%s> %s\r\n", stamp, prefix, text);
            break;
        case 4:
        case 8:
            fprintf(file, "%s %s\r\n", stamp, text);
            break;
    }
    fclose(file);
}

void CChatWindow::ResetPage()
{
    if(m_pScrollBar) m_pScrollBar->SetTrackPos(90);
}
