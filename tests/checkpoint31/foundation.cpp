// Integration fixtures, never counted as reconstructed R5 instructions.
#include <winsock2.h>
#include <stdio.h>
#include <stddef.h>
#include <math.h>
#include "../../vendor/upstream/saco/bass.h"
#pragma pack(push,1)
#include "../../vendor/upstream/raknet/BitStream.h"
#include "../../vendor/upstream/raknet/RakNetworkFactory.h"
#include "../../vendor/upstream/raknet/RakClientInterface.h"
#include "../../vendor/upstream/raknet/SHA1.h"
#include "../../vendor/upstream/raknet/CheckSum.h"
#include "../../vendor/upstream/saco/md5.h"
#include "../../vendor/upstream/saco/fhicl-sha1.h"
#pragma pack(pop)
#include "../../vendor/upstream/saco/d3d9/common/dxstdafx.h"

extern "C" int FoundationLayouts();
#include "hash_vectors.h"
static unsigned checks;
#define CHECK(x) do { ++checks; if(!(x)) {printf("FAIL foundation line %u: %s\n",__LINE__,#x);return 1;} } while(0)
static bool Digest(const unsigned char* bytes,const char* expected,unsigned count) {
    const char* digits="0123456789abcdef";
    for(unsigned i=0;i<count;++i)
        if(digits[bytes[i]>>4]!=expected[i*2] || digits[bytes[i]&15]!=expected[i*2+1]) return false;
    return true;
}
extern "C" __declspec(dllexport) int FoundationSelfTest() {
    CHECK(GetModuleHandleA("samp.dll")==0);
    CHECK(FoundationLayouts()==1);
    CHECK(sizeof(void*)==4 && sizeof(unsigned long)==4);
    CHECK(sizeof(PlayerID)==6 && offsetof(PlayerID,port)==4);
    CHECK(sizeof(RakNet::BitStream)==273);
    CHECK(sizeof(CheckSum)==10);
    CHECK(sizeof(CGrowableArray<int>)==12);
    CHECK(sizeof(DXUTBlendColor)==40 && offsetof(DXUTBlendColor,Current)==24);
    CHECK(sizeof(MD5Context)==88 && offsetof(MD5Context,in)==24);
    RakNet::BitStream stream;
    for(unsigned i=0;i<4096;++i) stream.Write((unsigned short)(i*17));
    CHECK(stream.GetNumberOfBytesUsed()==8192);
    for(unsigned j=0;j<4096;++j) {unsigned short value=0;CHECK(stream.Read(value));CHECK(value==(unsigned short)(j*17));}
    unsigned short extra=0;CHECK(!stream.Read(extra));
    stream.Reset();
    for(unsigned k=0;k<257;++k) stream.Write((k%3)==0);
    stream.ResetReadPointer();
    for(unsigned b=0;b<257;++b) {bool value=false;CHECK(stream.Read(value));CHECK(value==((b%3)==0));}
    stream.Reset();
    unsigned char pattern[4]={0xad,0x53,0x7f,0x91};
    stream.WriteBits(pattern,29,false);stream.ResetReadPointer();
    unsigned char result[4]={0,0,0,0};CHECK(stream.ReadBits(result,29,false));
    // R5 preserves unused low bits for left-aligned reads; do not "fix" them.
    CHECK(memcmp(result,pattern,4)==0 && stream.GetReadOffset()==29);
    stream.ResetReadPointer();CHECK(stream.ReadBits(result,29,true));
    CHECK(memcmp(result,pattern,3)==0 && result[3]==(pattern[3]>>3));
    RakClientInterface* client=RakNetworkFactory::GetRakClientInterface();
    CHECK(client!=0);CHECK(!client->IsConnected());
    client->SetPassword("foundation-local-test");
    CHECK(client->HasPassword());
    RakNetworkFactory::DestroyRakClientInterface(client);
    // No Connect, socket listener or network packet is initiated by this test.
    const char* messages[]={"","abc","The quick brown fox jumps over the lazy dog"};
    const char* md5[]={"d41d8cd98f00b204e9800998ecf8427e","900150983cd24fb0d6963f7d28e17f72","9e107d9d372bb6826bd81d3542a419d6"};
    const char* sha1[]={"da39a3ee5e6b4b0d3255bfef95601890afd80709","a9993e364706816aba3e25717850c26c9cd0d89d","2fd4e1c67a2d28fced849ee1bb76e7391b93eb12"};
    for(unsigned m=0;m<3;++m) {
        unsigned char digest[20];MD5Context md;
        MD5Init(&md);MD5Update(&md,(const unsigned char*)messages[m],strlen(messages[m]));MD5Final(digest,&md);
        CHECK(Digest(digest,md5[m],16));
        CSHA1 raksha;raksha.Update((unsigned char*)messages[m],strlen(messages[m]));raksha.Final();raksha.GetHash(digest);
        CHECK(Digest(digest,sha1[m],20));
        fhicl::SHA1 other;other.Input(messages[m],strlen(messages[m]));unsigned words[5];CHECK(other.Result(words));
        for(unsigned w=0;w<5;++w) for(unsigned byte=0;byte<4;++byte) digest[w*4+byte]=(unsigned char)(words[w]>>(24-byte*8));
        CHECK(Digest(digest,sha1[m],20));
    }
    unsigned char payload[1000];
    for(unsigned q=0;q<1000;++q) payload[q]=(unsigned char)q;
    for(unsigned vector=0;vector<sizeof(hashVectors)/sizeof(hashVectors[0]);++vector) {
        const HashVector& v=hashVectors[vector];unsigned char digest[20];MD5Context md;
        MD5Init(&md);CSHA1 raksha;fhicl::SHA1 other;
        // Deliberately stream chunks across 64-byte block boundaries.
        for(unsigned offset=0;offset<v.length;) {
            unsigned count=v.length-offset;if(count>17) count=17;
            MD5Update(&md,payload+offset,count);raksha.Update(payload+offset,count);
            other.Input(payload+offset,count);offset+=count;
        }
        MD5Final(digest,&md);CHECK(Digest(digest,v.md5,16));
        raksha.Final();raksha.GetHash(digest);CHECK(Digest(digest,v.sha1,20));
        unsigned words[5];CHECK(other.Result(words));
        for(unsigned word=0;word<5;++word) for(unsigned byte=0;byte<4;++byte)
            digest[word*4+byte]=(unsigned char)(words[word]>>(24-byte*8));
        CHECK(Digest(digest,v.sha1,20));
    }
    CGrowableArray<int> array;
    for(int a=0;a<1024;++a) CHECK(SUCCEEDED(array.Add(a*3)));
    CHECK(array.GetSize()==1024);CHECK(array.IndexOf(300)==100);
    CHECK(SUCCEEDED(array.Remove(100)));CHECK(array[100]==303);
    array.RemoveAll();CHECK(array.GetSize()==0);
    D3DXCOLOR begin(0.f,0.25f,0.5f,1.f),end(1.f,0.75f,0.5f,0.f),middle;
    D3DXColorLerp(&middle,&begin,&end,0.5f);
    CHECK(middle.r==0.5f && middle.g==0.5f && middle.b==0.5f && middle.a==0.5f);
    D3DXVECTOR3 direction(3.f,4.f,0.f),normalized;
    D3DXVec3Normalize(&normalized,&direction);
    CHECK(fabs(normalized.x-0.6f)<0.00001f && fabs(normalized.y-0.8f)<0.00001f);
    CHECK(GetModuleHandleA("d3dx9_25.dll")!=0);
    CHECK((BASS_GetVersion()>>16)==BASSVERSION);
    CHECK(GetModuleHandleA("bass.dll")!=0);
    DXUTBlendColor color;color.Init(0xff123456);
    CHECK(color.States[DXUT_STATE_NORMAL]==0xff123456);
    CDXUTTimer timer;timer.Reset();CHECK(!timer.IsStopped());timer.Stop();CHECK(timer.IsStopped());
    double previous=timer.GetTime();timer.Advance();CHECK(timer.GetTime()>previous);
    CHECK(GetModuleHandleA("samp.dll")==0);
    printf("PASS foundation: %u checks; network serialization, client lifecycle, hashes, UI utilities, ABI\n",checks);
    return 0;
}
BOOL WINAPI DllMain(HINSTANCE,DWORD,LPVOID) {return TRUE;}
