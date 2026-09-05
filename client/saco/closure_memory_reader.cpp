// Pointer-only bounded reader view from R5 constructor/read/seek field accesses.
// No objects allocated here; unknown field semantics do not imply whole-class coverage.
#include <windows.h>
#include <string.h>
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
//B3A30/38.
R5MemoryReaderView::R5MemoryReaderView(BYTE *data,unsigned int bytes)
{
    buffer=data;
    length=bytes;
    position=0;
    ended=false;
    unknown13=0;
    unknown22=0;
    unknown18=0;
    unknown14=0;
}
//BA7D0/126. Original zeroes requested output before clamping; preserves overflow behavior.
unsigned int R5MemoryReaderView::Read(void *output,unsigned int bytes)
{
    memset(output,0,bytes);
    if(position+bytes>length) bytes=length-position;
    if(bytes<=0) return 0;
    memcpy(output,buffer+position,bytes);
    Seek(bytes);
    return bytes;
}
float R5MemoryReaderView::ReadFloat()
{
    float result=0.0f;
    Read(&result,4);
    return result;
}
int R5MemoryReaderView::ReadInt()
{
    int result=0;
    Read(&result,4);
    return result;
}
BYTE R5MemoryReaderView::ReadByte()
{
    BYTE result=0;
    Read(&result,1);
    return result;
}
void R5MemoryReaderView::Reset()
{
    position=0;
    ended=false;
}
bool R5MemoryReaderView::Ended()
{
    return ended;
}

//B9530/27, full four-byte zeroed temporary then two-byte Read, WORD return.
WORD R5MemoryReaderView::ReadWord()
{
    int result=0;
    Read(&result,2);
    return (WORD)result;
}
