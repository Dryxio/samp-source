#include "main.h"
#include "game/util.h"
#include <time.h>
#define CHAT_TYPE_CHAT 1
#define MAX_LINE_LENGTH 64
#define MAX_MESSAGE_LENGTH 128
void CChatWindow::PushBack()
{
	int x=MAX_MESSAGES-1;
	while(x) {
		memcpy(&m_ChatWindowEntries[x],&m_ChatWindowEntries[x-1],sizeof(CHAT_WINDOW_ENTRY));
		x--;
	}
}
void CChatWindow::AddEntry(int eType, 
										PCHAR szString, 
										PCHAR szNick,
										DWORD dwTextColor,
										DWORD dwChatColor)
{
	int iBestLineLength=0;

	PushBack();

	m_ChatWindowEntries[0].type = eType;
	m_ChatWindowEntries[0].textColor = dwTextColor;
	m_ChatWindowEntries[0].prefixColor = dwChatColor;
	
	if(szNick) {
		strcpy(m_ChatWindowEntries[0].prefix,szNick);
		strcat(m_ChatWindowEntries[0].prefix,":");
	} else {
		m_ChatWindowEntries[0].prefix[0] = '\0';
	}

	if(m_ChatWindowEntries[0].type == CHAT_TYPE_CHAT && strlen(szString) > MAX_LINE_LENGTH)
	{
		iBestLineLength = MAX_LINE_LENGTH;
		// see if we can locate a space.
		while(szString[iBestLineLength] != ' ' && iBestLineLength)
			iBestLineLength--;

		if((MAX_LINE_LENGTH - iBestLineLength) > 12) {
			// we should just take the whole line
			strncpy(m_ChatWindowEntries[0].text,szString,MAX_LINE_LENGTH);
			m_ChatWindowEntries[0].text[MAX_LINE_LENGTH] = '\0';

			PushBack();
			
			m_ChatWindowEntries[0].type = eType;
			m_ChatWindowEntries[0].textColor = dwTextColor;
			m_ChatWindowEntries[0].prefixColor = dwChatColor;
			m_ChatWindowEntries[0].prefix[0] = '\0';

			strcpy(m_ChatWindowEntries[0].text,szString+MAX_LINE_LENGTH);
		}
		else {
			// grab upto the found space.
			strncpy(m_ChatWindowEntries[0].text,szString,iBestLineLength);
			m_ChatWindowEntries[0].text[iBestLineLength] = '\0';

			PushBack();

			m_ChatWindowEntries[0].type = eType;
			m_ChatWindowEntries[0].textColor = dwTextColor;
			m_ChatWindowEntries[0].prefixColor = dwChatColor;
			m_ChatWindowEntries[0].prefix[0] = '\0';

			strcpy(m_ChatWindowEntries[0].text,szString+(iBestLineLength+1));
		}
	}
	else {
		strncpy(m_ChatWindowEntries[0].text,szString,MAX_MESSAGE_LENGTH);
		m_ChatWindowEntries[0].text[MAX_MESSAGE_LENGTH] = '\0';
	}
	
}
