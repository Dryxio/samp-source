// R5 BA850/171 and BA900/137: complete23-byte memory-buffer view, no allocation of owner.
#include <windows.h>
#include <stdlib.h>
extern "C" int __stdcall LzmaCompress(unsigned char*,size_t*,const unsigned char*,size_t,unsigned char*,size_t*,int,unsigned,int,int,int,int,int);
extern "C" int __stdcall LzmaUncompress(unsigned char*,size_t*,const unsigned char*,size_t*,const unsigned char*,size_t);
unsigned char r5LzmaDefaultProperties[5]={0x5d,0,0,1,0};
class R5MemoryLzmaCodecView {
 BYTE *buffer;
 unsigned position,length;
 bool ended;
 BYTE ownsBuffer;
 BYTE *scratch;
 unsigned scratchLength;
 BYTE compressed;
public:
 int Compress();
 int Uncompress(unsigned outputSize);
};
int R5MemoryLzmaCodecView::Compress()
{
 unsigned char properties[5];
 size_t propertiesSize=5;
 unsigned originalLength=length;
 scratchLength=originalLength;
 if(compressed)return -1;
 scratch=(BYTE*)malloc(originalLength);
 int result=LzmaCompress(scratch,&scratchLength,buffer,originalLength,properties,&propertiesSize,1,65536,3,0,2,32,1);
 if(result==0){
  if(ownsBuffer)free(buffer);
  buffer=scratch;ownsBuffer=1;length=scratchLength;compressed=1;
 }else{free(scratch);compressed=0;}
 return result;
}
int R5MemoryLzmaCodecView::Uncompress(unsigned outputSize)
{
 if(!compressed)return -1;
 scratch=(BYTE*)malloc(outputSize);
 int result=LzmaUncompress(scratch,&outputSize,buffer,&length,r5LzmaDefaultProperties,5);
 if(result==0){
  if(ownsBuffer)free(buffer);
  ownsBuffer=1;buffer=scratch;length=outputSize;compressed=0;
 }else{free(scratch);compressed=1;}
 return result;
}
