// R5 reconstruction proposal, not compiled/accepted. Existing actual texture wrappers required.
#include <windows.h>
#include <string.h>
#include <stdio.h>
int FindTextureSlot(char *);
int AddTextureSlot(char *);
int LoadTexture(int, char *);
void AddTextureRef(int);
void PushCurrentTexture();
void PopCurrentTexture();
void SetCurrentTexture(int);
DWORD ReloadTexture(char *);
BOOL IsFileOrDirectoryExists(char *);

// B2B90/194. Local stack reservation 200 bytes: directory65 (rounded68) + filename129 (rounded132).
// Array dimensions inferred from stack lifetime; verify compiler output before acceptance.
BOOL EnsureTextdrawDictionary(char *dictionary, char *file, bool sampDirectory)
{
    char directory[65];
    char filename[129];
    if(sampDirectory) strcpy(directory, "SAMP\\");
    else strcpy(directory, "models\\txd");
    int slot = FindTextureSlot(dictionary);
    if(slot == -1) {
        slot = AddTextureSlot(dictionary);
        sprintf(filename, "%s\\%s.txd", directory, file);
        if(IsFileOrDirectoryExists(filename)) {
            LoadTexture(slot, filename);
            AddTextureRef(slot);
        } else {
            return FALSE;
        }
    }
    return TRUE;
}
// B2C60/60. Calls only accepted actual engine wrapper providers.
DWORD ReadTextdrawDictionaryTexture(char *dictionary, char *texture)
{
    int slot = FindTextureSlot(dictionary);
    if(slot == -1) return 0;
    PushCurrentTexture();
    SetCurrentTexture(slot);
    DWORD result = ReloadTexture(texture);
    PopCurrentTexture();
    return result;
}
