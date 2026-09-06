// Explicit complete symbolic exception: normal C++ trial1 produced138 bytes
// with only receiver/RwObject register allocation and save scheduling different.
// R5 9FB20/138. No opcode data, padding, aliases, tables or new absolute stores.
#include "main.h"
#include <stddef.h>
struct R5PreviewNativeEntityView {
    DWORD vtable;
    BYTE unknown4[0x14];
    void *rwObject;
    BYTE unknown1C[6];
    WORD model;
};
class R5PreviewEntityRenderView {
    BYTE unknown0[0x40];
    R5PreviewNativeEntityView *entity;
public:
    void Render();
};
__declspec(naked) void R5PreviewEntityRenderView::Render()
{
    enum {
        FrameBytes=12, VtableLocal=12, RwObjectLocal=8, EntityLocal=4,
        WrapperEntity=offsetof(R5PreviewEntityRenderView,entity),
        NativeVtable=offsetof(R5PreviewNativeEntityView,vtable),
        NativeRwObject=offsetof(R5PreviewNativeEntityView,rwObject),
        NativeModel=offsetof(R5PreviewNativeEntityView,model),
        NativePreRenderOffset=17*sizeof(void*),
        FirstVehicleModel=400, LastVehicleModel=611,
        NativeSetupVehicleVariables=0x733160,
        NativeRenderOneNonRoad=0x553260
    };
    __asm {
        push ebp
        mov ebp, esp
        sub esp, FrameBytes
        push esi
        mov esi, ecx
        mov eax, dword ptr [esi+WrapperEntity]
        mov ecx, dword ptr [eax+NativeVtable]
        mov dword ptr [ebp-VtableLocal], ecx
        mov ecx, eax
        test ecx, ecx
        push edi
        mov dword ptr [ebp-EntityLocal], ecx
        jne rw_object_present
        xor edi, edi
        jmp rw_object_selected
    rw_object_present:
        mov edi, dword ptr [eax+NativeRwObject]
    rw_object_selected:
        mov ax, word ptr [eax+NativeModel]
        cmp ax, FirstVehicleModel
        mov dword ptr [ebp-RwObjectLocal], edi
        jb prepare_render
        cmp ax, LastVehicleModel
        ja prepare_render
        test edi, edi
        je prepare_render
        push dword ptr [ebp-RwObjectLocal]
        mov edx, NativeSetupVehicleVariables
        call edx
        pop edx
    prepare_render:
        mov eax, dword ptr [ebp-VtableLocal]
        mov ecx, dword ptr [ebp-EntityLocal]
        call dword ptr [eax+NativePreRenderOffset]
        mov edx, dword ptr [esi+WrapperEntity]
        mov ax, word ptr [edx+NativeModel]
        cmp ax, FirstVehicleModel
        jb render_other_model
        cmp ax, LastVehicleModel
        ja render_other_model
        test edi, edi
        je finished
        push dword ptr [ebp-EntityLocal]
        mov edx, NativeRenderOneNonRoad
        call edx
        pop edx
        pop edi
        pop esi
        mov esp, ebp
        pop ebp
        ret
    render_other_model:
        test edi, edi
        je finished
        push dword ptr [ebp-EntityLocal]
        mov edx, NativeRenderOneNonRoad
        call edx
        pop edx
    finished:
        pop edi
        pop esi
        mov esp, ebp
        pop ebp
        ret
    }
}
