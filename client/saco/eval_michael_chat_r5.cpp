#include "main.h"
#include "game/util.h"
#include <time.h>
#define CHAT_TYPE_CHAT 2
#define MAX_LINE_LENGTH 72
#define MAX_MESSAGE_LENGTH 144
inline void CChatWindow::PushBack()
{
    memcpy(m_ChatWindowEntries, m_ChatWindowEntries + 1, sizeof(m_ChatWindowEntries));
}
void CChatWindow::AddEntry(int eType, 
										PCHAR szString, 
										PCHAR szNick,
										DWORD dwTextColor,
										DWORD dwChatColor)
{
	int iBestLineLength=0;

	PushBack();
	if(szNick && strlen(szNick) > 24) return;

	m_ChatWindowEntries[99].type = eType;
	m_ChatWindowEntries[99].textColor = dwTextColor;
	m_ChatWindowEntries[99].prefixColor = dwChatColor;
	
	m_ChatWindowEntries[99].timestamp = (DWORD)time(NULL);
	if(szNick) {
		strcpy(m_ChatWindowEntries[99].prefix,szNick);
		strcat(m_ChatWindowEntries[99].prefix,":");
	} else {
		m_ChatWindowEntries[99].prefix[0] = '\0';
	}

	Log(eType, szString, szNick);
	if(m_ChatWindowEntries[99].type == CHAT_TYPE_CHAT && strlen(szString) > MAX_LINE_LENGTH)
	{
		iBestLineLength = MAX_LINE_LENGTH;
		// see if we can locate a space.
		while(szString[iBestLineLength] != ' ' && iBestLineLength)
			iBestLineLength--;

		if((MAX_LINE_LENGTH - iBestLineLength) > 12) {
			// we should just take the whole line
			strncpy(m_ChatWindowEntries[99].text,szString,MAX_LINE_LENGTH);
			m_ChatWindowEntries[99].text[MAX_LINE_LENGTH] = '\0';

			PushBack();
			
			m_ChatWindowEntries[99].type = eType;
			m_ChatWindowEntries[99].textColor = dwTextColor;
			m_ChatWindowEntries[99].prefixColor = dwChatColor;
			m_ChatWindowEntries[99].prefix[0] = '\0';

			strcpy(m_ChatWindowEntries[99].text,szString+MAX_LINE_LENGTH);
		}
		else {
			// grab upto the found space.
			strncpy(m_ChatWindowEntries[99].text,szString,iBestLineLength);
			m_ChatWindowEntries[99].text[iBestLineLength] = '\0';

			PushBack();

			m_ChatWindowEntries[99].type = eType;
			m_ChatWindowEntries[99].textColor = dwTextColor;
			m_ChatWindowEntries[99].prefixColor = dwChatColor;
			m_ChatWindowEntries[99].prefix[0] = '\0';

			strcpy(m_ChatWindowEntries[99].text,szString+(iBestLineLength+1));
		}
	}
	else {
		strncpy(m_ChatWindowEntries[99].text,szString,MAX_MESSAGE_LENGTH);
		m_ChatWindowEntries[99].text[MAX_MESSAGE_LENGTH] = '\0';
	}
	

	field_63DA = 1;
}
