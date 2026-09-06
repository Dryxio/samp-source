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

// Pointer-only view of clump chunk slots, proven by dispatchIDs and stores.
class R5RwClumpDispatchView {
    DWORD error;
    BYTE unknown4[0x2c];
    R5RwChunk *structure;
    R5RwChunk *frameList;
    R5RwChunk *geometryList;
    R5RwChunk *atomic;
    R5RwChunk *extension;
public:
    BOOL SetStructure(R5RwChunk *chunk);
    BOOL SetFrameList(R5RwChunk *chunk);
    BOOL SetGeometryList(R5RwChunk *chunk);
    BOOL SetAtomic(R5RwChunk *chunk);
    BOOL SetExtension(R5RwChunk *chunk);
    BOOL ReadClumpChunks(R5RwChunk *clump);
};

class R5MemoryBufferView {
    BYTE *buffer;
    unsigned position;
    unsigned length;
    bool ended;
    BYTE ownsBuffer;
    DWORD reserved14;
    DWORD reserved18;
    BYTE reserved22;
public:
    bool MakeOwned();
    unsigned Write(const void *data,unsigned bytes);
    unsigned LoadFile(const char *filename);
};

// Pointer-only file parser state; all accessedoffsets verified, no parent allocation.
class R5RwClumpFileView {
    DWORD error;
    DWORD errorData;
    R5RwReaderStorage input;
    DWORD inputLength;
    BYTE hasVendorPlugin253F2FF;
    BYTE unknown24[8];
    R5RwChunk *root;
    BYTE unknown30[0x10];
    R5RwChunk *extension;
public:
    int LoadClumpFile(const char *filename);
    int LoadSingleClumpFile(const char *filename);
};
//B9920/185. Skip one UVAnimDict43, then require Clump16 and successful dispatch.
int R5RwClumpFileView::LoadClumpFile(const char *filename)
{
    if(!((R5MemoryBufferView*)&input)->LoadFile(filename)) return -1;
    inputLength=input.length;
    root=((R5RwFrameParserView*)this)->ReadChunk(&input);
    if(root) {
        if(root->type==43) {
            delete root;
            root=0;
            root=((R5RwFrameParserView*)this)->ReadChunk(&input);
        }
        // Original does not add a nullcheck after discarding the UVAnimDict.
        if(root->type==16) {
            if(!((R5RwClumpDispatchView*)this)->ReadClumpChunks(root)) return FALSE;
            if(extension && extension->length>0) {
                R5RwChunk *plugin=((R5RwFrameParserView*)this)->ReadChunk(&extension->body);
                if(plugin && plugin->type==0x253F2FF) hasVendorPlugin253F2FF=1;
                // Original does not free this child; no new ownership action.
            }
        } else {
            if(!error) error=0xABE0;
            return FALSE;
        }
    }
    return TRUE;
}
//B99E0/116. Original narrower path ignores dispatcher result and accepts nonclump root.
int R5RwClumpFileView::LoadSingleClumpFile(const char *filename)
{
    if(!((R5MemoryBufferView*)&input)->LoadFile(filename)) return -1;
    inputLength=input.length;
    root=((R5RwFrameParserView*)this)->ReadChunk(&input);
    if(root && root->type==16) {
        ((R5RwClumpDispatchView*)this)->ReadClumpChunks(root);
        if(extension && extension->length>0) {
            R5RwChunk *plugin=((R5RwFrameParserView*)this)->ReadChunk(&extension->body);
            if(plugin && plugin->type==0x253F2FF) hasVendorPlugin253F2FF=1;
        }
    }
    return TRUE;
}
