// Fixed-width descriptions for the x86 R5 ABI. No host-sized pointers in records.
// Evidence levels and binary witnesses live in config/checkpoint31/structures.json.
#pragma once
#pragma pack(push,1)
namespace R5 {
typedef unsigned char U8;
typedef unsigned short U16;
typedef unsigned int U32;
struct Vector3 { float x,y,z; };
struct Matrix { Vector3 right; U32 flags; Vector3 up; float padUp;
                Vector3 at; float padAt; Vector3 position; float padPosition; };
struct PlayerID { U32 address; U16 port; };
struct BitStream { int bitsUsed,bitsAllocated,readOffset; U32 data;
                   bool copyData; U8 stackData[256]; };
struct CheckSum { U16 r,c1,c2; U32 sum; };
struct Packet { U16 playerIndex; PlayerID playerId; U32 length,bitSize,data; bool deleteData; };
struct RPCParameters { U32 input,bits; PlayerID sender; U32 recipient,replyToSender; };
struct GrowableArray { U32 data; int size,capacity; };
struct BlendColor { U32 states[6]; float current[4]; };
struct MD5Context { U32 state[4],bits[2]; U8 block[64]; };
}
#pragma pack(pop)
