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
//BB70/35: original ownership flag chunk19 controls freeing chunkC.
R5RwChunk::~R5RwChunk()
{
    if(body.ownsBuffer && body.buffer) {
        free(body.buffer);
        body.buffer=0;
    }
}
//B9630/258. Entire35-byte allocated chunk storage is represented and initialized.

//B9F60/129, frame extension validator, preserving each original check/skip.

//B9FF0/299, frame records are full56-byte RenderWare wire records.
BOOL R5RwFrameParserView::ReadFrameList()
{
    R5RwChunkHeader header;
    if(!frameList) return FALSE;
    frameList->body.Reset();
    frameList->body.Reader()->Read(&header,12);
    if(header.type!=1) {
        if(!error) error=0xABE7;
        errorData=1;
        return FALSE;
    }
    unsigned count=frameList->body.Read32();
    if(count>=256) {
        if(!error) error=0xABEC;
        errorData=count;
        return FALSE;
    }
    frames=(R5RwFrameRecord*)calloc(count,sizeof(R5RwFrameRecord));
    frameList->body.Reader()->Read(frames,count*sizeof(R5RwFrameRecord));
    R5RwChunk *chunk=ReadChunk(&frameList->body);
    while(chunk) {
        if(chunk->type==3 && chunk->length>0) {
            if(!ValidateFrameExtensions(chunk)) {
                delete chunk;
                if(!error) error=0xABEE;
                return FALSE;
            }
        }
        delete chunk;
        chunk=ReadChunk(&frameList->body);
    }
    return TRUE;
}
