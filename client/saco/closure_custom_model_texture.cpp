// Whole A7B60/100 candidate. SetCustomModelTextureSlot remains required actualB4660.
#include <windows.h>
int FindTextureSlot(char *name);
int AddTextureSlot(char *name);
BOOL IsFileOrDirectoryExists(char *path);
int LoadTexture(int slot,char *path);
void PushCurrentTexture();
void SetCurrentTexture(int slot);
void __stdcall SetCustomModelTextureSlot(int model,int slot);
bool LoadCustomModelTexture(int model,char *dictionary,char *path)
{
    int slot=FindTextureSlot(dictionary);
    if(slot!=-1) {
        SetCustomModelTextureSlot(model,slot);
    } else {
        slot=AddTextureSlot(dictionary);
        if(IsFileOrDirectoryExists(path)) {
            LoadTexture(slot,path);
            SetCustomModelTextureSlot(model,slot);
        } else return false;
    }
    PushCurrentTexture();
    SetCurrentTexture(slot);
    return true;
}
