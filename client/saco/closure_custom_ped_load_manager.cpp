// C650/281 candidate; pointer-only view, full class size unknown.
#include <windows.h>
#include <stdio.h>
#include <stddef.h>
BOOL IsFileOrDirectoryExists(char *path);
int InstallCustomPedModel(int baseModel,int model,char *dictionary,char *dffPath,char *txdPath); // actual type1 A7BD0/81
struct R5CustomPedLoadEntryView {
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
class R5CustomPedLoadView {
    R5CustomPedLoadEntryView **items;
    unsigned int count;
    unsigned char unknown8;
    R5CustomPedLoadEntryView *GetAt(unsigned int index) { if(index>=count) return 0; return items[index]; }
    char primaryPath[261];
    char secondaryPath[261];
public:
    BOOL LoadPedModelFiles(int baseModel,int model,DWORD dffChecksum,DWORD txdChecksum);
    BOOL EnsurePedModelReady(int model);
};

// Symbolic resolver adapted from accepted C770 after its three normal C++
// layout failures; this distinct C650 path calls the true PED installer A7BD0.
// Literal providers are complete original format strings, not instruction data.
extern const char r5CustomModelDictionaryFormat[];
extern const char r5CustomModelDffFormat[];
extern const char r5CustomModelTxdFormat[];
__declspec(naked) BOOL R5CustomPedLoadView::LoadPedModelFiles(int baseModel,int model,DWORD dffChecksum,DWORD txdChecksum)
{
    enum {
        LocalBytes=0x22c,
        DictionaryLocal=0x10, TxdLocal=0x2c, DffLocal=0x134,
        ArgBase=0x240, ArgModel=0x244, ArgDff=0x248, ArgTxd=0x24c,
        PrimaryPath=offsetof(R5CustomPedLoadView,primaryPath),
        SecondaryPath=offsetof(R5CustomPedLoadView,secondaryPath)
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
        call InstallCustomPedModel
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
// D2C0/94, type1 branch. No present/downloaded gate in original.
BOOL R5CustomPedLoadView::EnsurePedModelReady(int model)
{
    unsigned int index=0;
    while(index!=count) {
        R5CustomPedLoadEntryView *entry=GetAt(index);
        if(entry->modelId==model && entry->type==1) {
            if(LoadPedModelFiles(entry->baseModel,entry->modelId,entry->dffChecksum,entry->txdChecksum)) {
                entry->retry=false;
                entry->ready=true;
                return TRUE;
            }
            return FALSE;
        }
        index++;
    }
    return FALSE;
}
