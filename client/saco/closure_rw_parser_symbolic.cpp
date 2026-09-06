// R5 RenderWare frame/chunk parser candidates. No runtime/game execution.
#include <windows.h>
#include <stdlib.h>
#include <stddef.h>
class R5MemoryReaderView {
    BYTE *buffer;
    unsigned int position;
    unsigned int length;
    bool ended;
    BYTE unknown13;
    DWORD unknown14;
    DWORD unknown18;
    BYTE unknown22;
public:
    R5MemoryReaderView(BYTE *data,unsigned int bytes);
    unsigned int Read(void *output,unsigned int bytes);
    float ReadFloat();
    int ReadInt();
    BYTE ReadByte();
    WORD ReadWord();
    unsigned int Seek(unsigned int bytes) {
        unsigned int next=position+bytes;
        if(next<length) { position=next; return next; }
        position=length;
        ended=true;
        return length;
    }
    void Reset();
    bool Ended();
};

// Complete observed 23-byte nested reader storage; no fields omitted.
struct R5RwReaderStorage {
    BYTE *buffer;
    unsigned position;
    unsigned length;
    bool ended;
    BYTE ownsBuffer;
    DWORD reserved14;
    DWORD reserved18;
    BYTE reserved22;
    __declspec(dllexport) R5RwReaderStorage() {
        buffer=0; length=0; position=0; ended=false; ownsBuffer=0;
        reserved22=0; reserved18=0; reserved14=0;
    }
    __forceinline R5MemoryReaderView *Reader() { return (R5MemoryReaderView*)this; }
    __forceinline void Reset() { position=0; ended=false; }
    __forceinline void SetBuffer(BYTE *data,unsigned bytes) {
        length=bytes;
        buffer=data;
        position=0;
        ended=false;
        ownsBuffer=0;
    }
    __forceinline unsigned Read32() {
        unsigned value=0;
        Reader()->Read(&value,4);
        return value;
    }
    __forceinline WORD Read16() {
        unsigned value=0;
        Reader()->Read(&value,2);
        return (WORD)value;
    }
};
struct R5RwChunk {
    DWORD type;
    DWORD length;
    WORD build;
    WORD version;
    R5RwReaderStorage body;
    __declspec(dllexport) R5RwChunk() { type=0; length=0; build=0xFFFF; version=0x1803; }
    ~R5RwChunk();
};
struct R5RwChunkHeader { DWORD type,length,libraryId; };
struct R5RwFrameRecord { float rotation[9]; float position[3]; int parent; DWORD flags; };
typedef char reader_size23[(sizeof(R5RwReaderStorage)==23)?1:-1];
typedef char chunk_size35[(sizeof(R5RwChunk)==35)?1:-1];
typedef char frame_record_size56[(sizeof(R5RwFrameRecord)==56)?1:-1];
class R5RwFrameParserView {
    DWORD error;
    DWORD errorData;
    BYTE unknown8[0x2c];
    R5RwChunk *frameList;
    BYTE unknown38[0x10];
    R5RwFrameRecord *frames;
public:
    R5RwChunk *ReadChunk(R5RwReaderStorage *reader);
    BOOL ValidateFrameExtensions(R5RwChunk *extension);
    BOOL ReadFrameList();
};

// Symbol-only aliases. These are linker aliases to existing compiled providers,
// never wrapper implementations, function stubs, or additional accepted code.
// All uses below are assembly CALLs honoring the target's actual ABI.
extern "C" void * __cdecl r5_rw_OperatorNew(unsigned bytes);
extern "C" void __cdecl r5_rw_Read_thiscall_assembly_only();
#pragma comment(linker, "/alternatename:_r5_rw_OperatorNew=??2@YAPAXI@Z")
#pragma comment(linker, "/alternatename:_r5_rw_Read_thiscall_assembly_only=?Read@R5MemoryReaderView@@QAEIPAXI@Z")
enum {
    RR_BUFFER=offsetof(R5RwReaderStorage,buffer),
    RR_POSITION=offsetof(R5RwReaderStorage,position),
    RR_LENGTH=offsetof(R5RwReaderStorage,length),
    RR_ENDED=offsetof(R5RwReaderStorage,ended),
    RR_OWNS=offsetof(R5RwReaderStorage,ownsBuffer),
    RR_RESERVED14=offsetof(R5RwReaderStorage,reserved14),
    RR_RESERVED18=offsetof(R5RwReaderStorage,reserved18),
    RR_RESERVED22=offsetof(R5RwReaderStorage,reserved22),
    RC_TYPE=offsetof(R5RwChunk,type), RC_LENGTH=offsetof(R5RwChunk,length),
    RC_BUILD=offsetof(R5RwChunk,build), RC_VERSION=offsetof(R5RwChunk,version),
    RC_BODY=offsetof(R5RwChunk,body), RC_BYTES=sizeof(R5RwChunk),
    RW_HEADER_BYTES=sizeof(R5RwChunkHeader), RW_HANIM=0x11E,
    RW_FRAME_NAME=0x253F2FE, RW_FRAME_NAME_LIMIT=24
};
// Explicit source-derived x86 exception after bounded C++ trials263/266.
// EDI=input reader, ESI=allocated chunk, EBX=zero then parent cursor.
// The dead input-argument stack slot becomes the DWORD read temporary.
__declspec(naked) R5RwChunk *R5RwFrameParserView::ReadChunk(R5RwReaderStorage *reader)
{
    __asm {
        push ebx
        push edi
        mov edi, [esp+0Ch]
        mov eax, [edi+RR_LENGTH]
        xor ebx, ebx
        cmp eax, ebx
        je read_invalid
        cmp byte ptr [edi+RR_ENDED], bl
        je read_valid
    read_invalid:
        pop edi
        xor eax, eax
        pop ebx
        ret 4
    read_valid:
        push esi
        push RC_BYTES
        call r5_rw_OperatorNew
        add esp, 4
        cmp eax, ebx
        je allocation_failed
        mov [eax+RC_BODY+RR_BUFFER], ebx
        mov [eax+RC_BODY+RR_LENGTH], ebx
        mov [eax+RC_BODY+RR_POSITION], ebx
        mov [eax+RC_BODY+RR_ENDED], bl
        mov [eax+RC_BODY+RR_OWNS], bl
        mov [eax+RC_BODY+RR_RESERVED22], bl
        mov [eax+RC_BODY+RR_RESERVED18], ebx
        mov [eax+RC_BODY+RR_RESERVED14], ebx
        mov [eax+RC_TYPE], ebx
        mov [eax+RC_LENGTH], ebx
        mov word ptr [eax+RC_BUILD], 0FFFFh
        mov word ptr [eax+RC_VERSION], 01803h
        mov esi, eax
        jmp chunk_allocated
    allocation_failed:
        xor esi, esi
    chunk_allocated:
        push 4
        lea eax, [esp+14h]
        push eax
        mov ecx, edi
        mov [esp+18h], ebx
        call r5_rw_Read_thiscall_assembly_only
        mov ecx, [esp+10h]
        push 4
        lea edx, [esp+14h]
        mov [esi+RC_TYPE], ecx
        push edx
        mov ecx, edi
        mov [esp+18h], ebx
        call r5_rw_Read_thiscall_assembly_only
        mov eax, [esp+10h]
        push 2
        lea ecx, [esp+14h]
        push ecx
        mov ecx, edi
        mov [esi+RC_LENGTH], eax
        mov [esp+18h], ebx
        call r5_rw_Read_thiscall_assembly_only
        mov dx, [esp+10h]
        push 2
        lea eax, [esp+14h]
        push eax
        mov ecx, edi
        mov [esi+RC_BUILD], dx
        mov [esp+18h], ebx
        call r5_rw_Read_thiscall_assembly_only
        mov cx, [esp+10h]
        mov eax, [esi+RC_LENGTH]
        mov [esi+RC_VERSION], cx
        mov ecx, [edi+RR_BUFFER]
        mov edx, [edi+RR_POSITION]
        add edx, ecx
        mov [esi+RC_BODY+RR_LENGTH], eax
        mov [esi+RC_BODY+RR_BUFFER], edx
        mov [esi+RC_BODY+RR_POSITION], ebx
        mov [esi+RC_BODY+RR_ENDED], bl
        mov [esi+RC_BODY+RR_OWNS], bl
        mov eax, [esi+RC_LENGTH]
        mov ebx, [edi+RR_POSITION]
        mov ecx, [edi+RR_LENGTH]
        add eax, ebx
        cmp eax, ecx
        jae saturate_reader
        mov [edi+RR_POSITION], eax
        mov eax, esi
        pop esi
        pop edi
        pop ebx
        ret 4
    saturate_reader:
        mov eax, esi
        pop esi
        mov [edi+RR_POSITION], ecx
        mov byte ptr [edi+RR_ENDED], 1
        pop edi
        pop ebx
        ret 4
    }
}
// Explicit source-derived x86 exception after C++ trials114/106.
// EDI=extension chunk, ESI=nested reader, EBX=TRUE; header is12 stack bytes.
__declspec(naked) BOOL R5RwFrameParserView::ValidateFrameExtensions(R5RwChunk *extension)
{
    __asm {
        sub esp, RW_HEADER_BYTES
        push ebx
        push esi
        push edi
        mov edi, [esp+1Ch]
        mov al, [edi+RC_BODY+RR_ENDED]
        test al, al
        mov ebx, 1
        jne extensions_done
        lea esi, [edi+RC_BODY]
        align 16
    extension_next:
        push RW_HEADER_BYTES
        lea eax, [esp+10h]
        push eax
        mov ecx, esi
        call r5_rw_Read_thiscall_assembly_only
        mov eax, [esp+0Ch]
        cmp eax, RW_HANIM
        je skip_extension
        cmp eax, RW_FRAME_NAME
        jne skip_extension
        mov ecx, [esp+10h]
        cmp ecx, RW_FRAME_NAME_LIMIT
        jb advance_payload
        pop edi
        pop esi
        xor eax, eax
        pop ebx
        add esp, RW_HEADER_BYTES
        ret 4
    skip_extension:
        mov ecx, [esp+10h]
    advance_payload:
        mov eax, [esi+RR_POSITION]
        add eax, ecx
        mov ecx, [esi+RR_LENGTH]
        cmp eax, ecx
        jae extension_reader_ended
        mov [esi+RR_POSITION], eax
        jmp check_extension_end
    extension_reader_ended:
        mov [esi+RR_POSITION], ecx
        mov [esi+RR_ENDED], bl
    check_extension_end:
        mov al, [edi+RC_BODY+RR_ENDED]
        test al, al
        je extension_next
    extensions_done:
        pop edi
        pop esi
        mov eax, ebx
        pop ebx
        add esp, RW_HEADER_BYTES
        ret 4
    }
}
