// New R5 reconstruction; no 0.2.5 implementation exists for d3dhook.
#include <windows.h>
#include <string.h>
extern DWORD GetColorFromEmbedCode(wchar_t *);
extern DWORD r5FontEmbeddedColor;             // R5 0x136E90, DWORD
extern int r5FontCharacterIndex;              // R5 0x136EA4, int
extern wchar_t r5FontWideText[20000];          // R5 0x136EA8, complete 40000 bytes
extern char *r5FontOriginalText;               // R5 0x140AEC; startup owns allocation

// R5 0x739A0/108. Capacity includes the cleared terminator space.
void R5FontConvertAnsiToWide(char *source, wchar_t *destination, int capacity)
{
    memset(destination, 0, capacity * sizeof(wchar_t));
    int required = MultiByteToWideChar(CP_ACP, 0, source, strlen(source), NULL, 0);
    if (required < capacity)
        MultiByteToWideChar(CP_ACP, 0, source, strlen(source), destination, required);
}

// R5 0x73AE0/223. A glyph draw consumes preceding whitespace/color embeds.
// The final guard genuinely indexes the ANSI copy using the wide cursor in R5.
// Preserve this behavior, including the unchanged cursor at terminator.
void R5FontConsumeWideColorEmbeds()
{
    while (r5FontWideText[r5FontCharacterIndex] &&
          (r5FontWideText[r5FontCharacterIndex] == L' ' ||
           r5FontWideText[r5FontCharacterIndex] == L'\t' ||
           r5FontWideText[r5FontCharacterIndex] == L'\n' ||
           r5FontWideText[r5FontCharacterIndex] == L'\r'))
        ++r5FontCharacterIndex;
    DWORD color = GetColorFromEmbedCode(r5FontWideText + r5FontCharacterIndex);
    while (color != 0xFFFFFFFF)
    {
        r5FontEmbeddedColor = color | 0xFF000000;
        r5FontCharacterIndex += 8;
        while (r5FontWideText[r5FontCharacterIndex] &&
              (r5FontWideText[r5FontCharacterIndex] == L' ' ||
               r5FontWideText[r5FontCharacterIndex] == L'\t' ||
               r5FontWideText[r5FontCharacterIndex] == L'\n' ||
               r5FontWideText[r5FontCharacterIndex] == L'\r'))
            ++r5FontCharacterIndex;
        color = GetColorFromEmbedCode(r5FontWideText + r5FontCharacterIndex);
    }
    if (r5FontOriginalText[r5FontCharacterIndex])
        ++r5FontCharacterIndex;
}
