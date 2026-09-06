// R5 type1 custom ped model path, distinct from accepted type2 object path.
#include <windows.h>
#include <stdlib.h>
#include <string.h>
// Exact existing provider body, exposed in this TU to retain its real clobber set.
// Sole GetModelInfo definition; closure_modelinfo retains only its declaration.
extern BYTE **extendedModelInfos;
extern BOOL useExtendedModelInfos;
BYTE *__stdcall GetModelInfo(int model)
{
    if(useExtendedModelInfos) return extendedModelInfos[model];
    if(model < 0 || model > 20000) return NULL;
    return ((BYTE **)0xA9B0C8)[model];
}
BOOL __stdcall IsActorPedModelValid(int);
extern BYTE **extendedModelInfos;
bool LoadCustomModelTexture(int,char*,char*);
void *ReadPedModelClump(char *path);
void __stdcall SetPedModelClump(int model,void *clump);
BYTE *ClonePedModelInfo(int sourceModel,int destinationModel)
{
    BYTE *copy=(BYTE*)calloc(68,1);
    BYTE *source=GetModelInfo(sourceModel);
    if(source && IsActorPedModelValid(sourceModel)) {
        memcpy(copy,source,68);
        extendedModelInfos[destinationModel]=copy;
        return copy;
    }
    return NULL;
}
int InstallCustomPedModel(int baseModel,int model,char *dictionary,char *dffPath,char *txdPath)
{
    if(ClonePedModelInfo(baseModel,model)) {
        if(LoadCustomModelTexture(model,dictionary,txdPath)) {
            void *clump=ReadPedModelClump(dffPath);
            if(clump) {
                SetPedModelClump(model,clump);
                return 1;
            }
        }
    }
    return 0;
}
