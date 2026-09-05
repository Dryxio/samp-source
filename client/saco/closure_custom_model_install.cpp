// A7C30/133 whole candidate. Uses actual pChatWindow and actual model foundation providers.
#include "main.h"
extern CChatWindow *pChatWindow;
extern int r5InstallingModel;
BYTE *CloneCustomModelInfo(int,int);
bool LoadCustomModelTexture(int,char*,char*);
int LoadCustomModelClump(char*,int);
int InstallCustomModel(int baseModel,int model,char *dictionary,char *dffPath,char *txdPath)
{
    r5InstallingModel=model;
    if(CloneCustomModelInfo(baseModel,model)) {
        if(LoadCustomModelTexture(model,dictionary,txdPath)) {
            if(LoadCustomModelClump(dffPath,model)) return 1;
            if(pChatWindow) pChatWindow->AddDebugMessage("[error] Failed to load model ID: %d",model);
        } else {
            if(pChatWindow) pChatWindow->AddDebugMessage("[error] Failed to load texture for model ID: %d",model);
        }
    }
    return 0;
}
