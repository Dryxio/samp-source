// R5 memory-backed file reader/writer, whole observed23-byte field view.
// No reader objects constructed; actual payload malloc/realloc/free preserve original behavior.
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
//BA710/57, detach borrowed payload by copying all existinglength bytes.
bool R5MemoryBufferView::MakeOwned()
{
    BYTE *owned=(BYTE*)malloc(length);
    if(!owned) return false;
    memcpy(owned,buffer,length);
    buffer=owned;
    ownsBuffer=1;
    return true;
}
//BA750/116. FailedMakeOwned/realloc handling intentionally unchanged from R5.
unsigned R5MemoryBufferView::Write(const void *data,unsigned bytes)
{
    if(position+bytes>length) {
        if(!ownsBuffer) MakeOwned();
        unsigned extra=bytes-length+position;
        buffer=(BYTE*)realloc(buffer,length+extra);
        length+=extra;
    }
    memcpy(buffer+position,data,bytes);
    position+=bytes;
    return bytes;
}
//BA990/187, actual2048-bytefile chunks; FILE::_flag ABI matches pinnedCRT.
unsigned R5MemoryBufferView::LoadFile(const char *filename)
{
    BYTE block[2048];
    position=0;
    if(ownsBuffer && buffer) {
        free(buffer);
        buffer=0;
    }
    FILE *file=fopen(filename,"rb");
    if(!file) return 0;
    ownsBuffer=1;
    buffer=(BYTE*)malloc(1);
    length=1;
    while(!(file->_flag & _IOEOF)) {
        unsigned count=fread(block,1,sizeof(block),file);
        Write(block,count);
    }
    fclose(file);
    position=0;
    ended=false;
    return length;
}
