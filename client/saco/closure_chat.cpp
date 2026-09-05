// Generated complete definitions from chatwindow.cpp; see tools/prepare_actor_closure.py.
#include <time.h>
#include <math.h>
#include "main.h"
#include "game/util.h"


inline void CChatWindow::PushBack()
{
    memcpy(m_ChatWindowEntries, m_ChatWindowEntries + 1, sizeof(m_ChatWindowEntries));
}

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
