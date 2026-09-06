#include <windows.h>
#include <stdlib.h>
extern DWORD GetColorFromEmbedCode(char *);
extern DWORD r5FontEmbeddedColor;
extern int r5FontCharacterIndex;
extern char *r5FontOriginalText;
// R5 73A10/201: ANSI counterpart, consumes one multibyte character via mblen.
// No original callers asserted; the actual proxy uses the wide counterpart.
void R5FontConsumeAnsiColorEmbeds()
{
    while (r5FontOriginalText[r5FontCharacterIndex] &&
          (r5FontOriginalText[r5FontCharacterIndex] == ' ' ||
           r5FontOriginalText[r5FontCharacterIndex] == '\t' ||
           r5FontOriginalText[r5FontCharacterIndex] == '\n' ||
           r5FontOriginalText[r5FontCharacterIndex] == '\r'))
        ++r5FontCharacterIndex;
    DWORD color = GetColorFromEmbedCode(r5FontOriginalText + r5FontCharacterIndex);
    while (color != 0xFFFFFFFF)
    {
        r5FontEmbeddedColor = color | 0xFF000000;
        r5FontCharacterIndex += 8;
        while (r5FontOriginalText[r5FontCharacterIndex] &&
              (r5FontOriginalText[r5FontCharacterIndex] == ' ' ||
               r5FontOriginalText[r5FontCharacterIndex] == '\t' ||
               r5FontOriginalText[r5FontCharacterIndex] == '\n' ||
               r5FontOriginalText[r5FontCharacterIndex] == '\r'))
            ++r5FontCharacterIndex;
        color = GetColorFromEmbedCode(r5FontOriginalText + r5FontCharacterIndex);
    }
    if (r5FontOriginalText[r5FontCharacterIndex])
    {
        int length = mblen(r5FontOriginalText + r5FontCharacterIndex, MB_CUR_MAX);
        if (length < 0)
            ++r5FontCharacterIndex;
        else
            r5FontCharacterIndex += length;
    }
}
