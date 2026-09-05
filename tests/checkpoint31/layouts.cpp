// Compile-time ABI checks against actual library declarations, on MSVC x86.
#include <winsock2.h>
#include <stddef.h>
#define private public
#define protected public
#pragma pack(push,1)
#include "../../vendor/upstream/raknet/BitStream.h"
#include "../../vendor/upstream/raknet/NetworkTypes.h"
#include "../../vendor/upstream/raknet/CheckSum.h"
#pragma pack(pop)
#include "../../vendor/upstream/saco/d3d9/common/dxstdafx.h"
#undef private
#undef protected
#include "../../vendor/upstream/saco/md5.h"
#include "../../src/foundation/layouts.h"
#define VERIFY(name,condition) typedef char verify_##name[(condition)?1:-1]
VERIFY(pointer,sizeof(void*)==4);
VERIFY(bitstream,sizeof(R5::BitStream)==sizeof(RakNet::BitStream));
VERIFY(bits_used,offsetof(RakNet::BitStream,numberOfBitsUsed)==0);
VERIFY(bits_allocated,offsetof(RakNet::BitStream,numberOfBitsAllocated)==4);
VERIFY(read_offset,offsetof(RakNet::BitStream,readOffset)==8);
VERIFY(data,offsetof(RakNet::BitStream,data)==12);
VERIFY(copy,offsetof(RakNet::BitStream,copyData)==16);
VERIFY(stack,offsetof(RakNet::BitStream,stackData)==17);
VERIFY(checksum,sizeof(R5::CheckSum)==sizeof(CheckSum) && offsetof(CheckSum,sum)==6);
VERIFY(playerid,sizeof(R5::PlayerID)==sizeof(PlayerID) && offsetof(PlayerID,port)==4);
VERIFY(packet,sizeof(R5::Packet)==sizeof(Packet) && offsetof(Packet,data)==16);
VERIFY(rpc,sizeof(R5::RPCParameters)==sizeof(RPCParameters) && offsetof(RPCParameters,recipient)==14);
VERIFY(array,sizeof(R5::GrowableArray)==sizeof(CGrowableArray<int>));
VERIFY(blend,sizeof(R5::BlendColor)==sizeof(DXUTBlendColor) && offsetof(DXUTBlendColor,Current)==24);
VERIFY(md5,sizeof(R5::MD5Context)==sizeof(MD5Context) && offsetof(MD5Context,bits)==16);
VERIFY(vector,sizeof(R5::Vector3)==12);
VERIFY(matrix,sizeof(R5::Matrix)==64 && offsetof(R5::Matrix,position)==48);
#define FIELD(name,view,member,actual,field) VERIFY(name,offsetof(view,member)==offsetof(actual,field))
FIELD(mirror_bits,R5::BitStream,bitsUsed,RakNet::BitStream,numberOfBitsUsed);
FIELD(mirror_alloc,R5::BitStream,bitsAllocated,RakNet::BitStream,numberOfBitsAllocated);
FIELD(mirror_read,R5::BitStream,readOffset,RakNet::BitStream,readOffset);
FIELD(mirror_data,R5::BitStream,data,RakNet::BitStream,data);
FIELD(mirror_copy,R5::BitStream,copyData,RakNet::BitStream,copyData);
FIELD(mirror_stack,R5::BitStream,stackData,RakNet::BitStream,stackData);
FIELD(mirror_sum,R5::CheckSum,sum,CheckSum,sum);
FIELD(mirror_port,R5::PlayerID,port,PlayerID,port);
FIELD(mirror_packet,R5::Packet,data,Packet,data);
FIELD(mirror_rpc,R5::RPCParameters,recipient,RPCParameters,recipient);
FIELD(mirror_blend,R5::BlendColor,current,DXUTBlendColor,Current);
FIELD(mirror_md5,R5::MD5Context,bits,MD5Context,bits);
extern "C" int FoundationLayouts() {return 1;}
