// C770/281 candidate; pointer-only view, full class size unknown.
#include <windows.h>
#include <stdio.h>
#include <stddef.h>
BOOL IsFileOrDirectoryExists(char *path);
int InstallCustomModel(int baseModel,int model,char *dictionary,char *dffPath,char *txdPath); // actual accepted A7C30/133
struct R5CustomModelLoadEntryView {
    BYTE present;
    BYTE downloaded;
    unsigned char unknown2[5];
    BYTE type;
    unsigned char unknown8[4];
    int baseModel;
    int modelId;
    unsigned char unknown14[0x32];
    DWORD dffChecksum;
    DWORD txdChecksum;
    unsigned char unknown4e[10];
    bool ready;
    bool retry;
};
class R5CustomModelLoadView {
    R5CustomModelLoadEntryView **items;
    unsigned int count;
    unsigned char unknown8;
    R5CustomModelLoadEntryView *GetAt(unsigned int index) { return index<count ? items[index] : 0; }
    char primaryPath[261];
    char secondaryPath[261];
public:
    BOOL LoadModelFiles(int baseModel,int model,DWORD dffChecksum,DWORD txdChecksum);
    BOOL EnsureModelReady(int model);
};

// Explicit symbolic exception authorized after three normal C++ attempts.
// Literal providers are complete original format strings, not instruction data.
extern const char r5CustomModelDictionaryFormat[];
extern const char r5CustomModelDffFormat[];
extern const char r5CustomModelTxdFormat[];
__declspec(naked) BOOL R5CustomModelLoadView::LoadModelFiles(int baseModel,int model,DWORD dffChecksum,DWORD txdChecksum)
{
    enum {
        LocalBytes=0x22c,
        DictionaryLocal=0x10, TxdLocal=0x2c, DffLocal=0x134,
        ArgBase=0x240, ArgModel=0x244, ArgDff=0x248, ArgTxd=0x24c,
        PrimaryPath=offsetof(R5CustomModelLoadView,primaryPath),
        SecondaryPath=offsetof(R5CustomModelLoadView,secondaryPath)
    };
    __asm {
        sub esp, LocalBytes
        push ebx
        mov ebx, dword ptr [esp+ArgTxd-12]
        push ebp
        push esi
        push edi
        push ebx
        lea eax, [esp+DictionaryLocal+4]
        push offset r5CustomModelDictionaryFormat
        push eax
        mov esi, ecx
        call sprintf
        mov ebp, dword ptr [esp+ArgDff+12]
        push ebp
        lea edi, [esi+PrimaryPath]
        push edi
        lea ecx, [esp+DffLocal+20]
        push offset r5CustomModelDffFormat
        push ecx
        call sprintf
        lea edx, [esp+DffLocal+28]
        push edx
        call IsFileOrDirectoryExists
        add esp, 32
        test eax, eax
        jne resolve_txd
        push ebp
        lea eax, [esi+SecondaryPath]
        push eax
        lea ecx, [esp+DffLocal+8]
        push offset r5CustomModelDffFormat
        push ecx
        call sprintf
        lea edx, [esp+DffLocal+16]
        push edx
        call IsFileOrDirectoryExists
        add esp, 20
        test eax, eax
        jne resolve_txd
missing_file:
        pop edi
        pop esi
        pop ebp
        xor eax, eax
        pop ebx
        add esp, LocalBytes
        ret 16
resolve_txd:
        push ebx
        push edi
        lea eax, [esp+TxdLocal+8]
        push offset r5CustomModelTxdFormat
        push eax
        call sprintf
        lea ecx, [esp+TxdLocal+16]
        push ecx
        call IsFileOrDirectoryExists
        add esp, 20
        test eax, eax
        jne install_model
        push ebx
        add esi, SecondaryPath
        push esi
        lea edx, [esp+TxdLocal+8]
        push offset r5CustomModelTxdFormat
        push edx
        call sprintf
        lea eax, [esp+TxdLocal+16]
        push eax
        call IsFileOrDirectoryExists
        add esp, 20
        test eax, eax
        je missing_file
install_model:
        lea ecx, [esp+TxdLocal]
        push ecx
        mov ecx, dword ptr [esp+ArgModel+4]
        lea edx, [esp+DffLocal+4]
        push edx
        mov edx, dword ptr [esp+ArgBase+8]
        lea eax, [esp+DictionaryLocal+8]
        push eax
        push ecx
        push edx
        call InstallCustomModel
        add esp, 20
        pop edi
        neg eax
        pop esi
        sbb eax, eax
        pop ebp
        neg eax
        pop ebx
        add esp, LocalBytes
        ret 16
    }
}
// D320/105; original flagsandcall checked beforesettingready, nofakeallocator.
BOOL R5CustomModelLoadView::EnsureModelReady(int model)
{
    unsigned int x=0;
    while(x!=count) {
        R5CustomModelLoadEntryView *entry=GetAt(x);
        if(entry && entry->modelId==model && entry->type==2) {
            if(!entry->present || !entry->downloaded) return FALSE;
            if(LoadModelFiles(entry->baseModel,entry->modelId,entry->dffChecksum,entry->txdChecksum)) {
                entry->retry=false;
                entry->ready=true;
                return TRUE;
            }
            return FALSE;
        }
        x++;
    }
    return FALSE;
}
