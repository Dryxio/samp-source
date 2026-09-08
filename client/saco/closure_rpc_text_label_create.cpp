#include "main.h"
#include <stddef.h>
extern DWORD unnamed_100B6100(char*,int);
#pragma pack(push,1)
struct R5CompleteLabelRecord {
 char *text;
 DWORD color;
 VECTOR position;
 float drawDistance;
 bool testLOS;
 WORD playerId;
 WORD vehicleId;
};
class R5CompleteLabelPool {
public:
 R5CompleteLabelRecord labels[MAX_LABELS];
 BOOL slots[MAX_LABELS];
 void New(WORD id,char *text,DWORD color,float x,float y,float z,float distance,bool testLOS,WORD playerId,WORD vehicleId);
 void Draw();
};
#pragma pack(pop)
typedef char LabelRecordSize[(sizeof(R5CompleteLabelRecord)==29)?1:-1];
typedef char LabelPlayerOffset[(offsetof(R5CompleteLabelRecord,playerId)==25)?1:-1];
typedef char LabelPoolSlotsOffset[(offsetof(R5CompleteLabelPool,slots)==0xE800)?1:-1];
typedef char LabelPoolSize[(sizeof(R5CompleteLabelPool)==0x10800)?1:-1];
#include "../raknet/StringCompressor.h"
extern CNetGame *pNetGame;
struct R5LabelRPCNetView { BYTE prefix[0x3de]; NETGAME_POOLS *pools; };
void CreateTextLabelRPCTransfer(RPCParameters *rpcParams) {
 PCHAR data=(PCHAR)rpcParams->input;
 int bits=rpcParams->numberOfBitsOfData;
 RakNet::BitStream stream(data,bits/8+1,false);
 R5CompleteLabelPool *pool=(R5CompleteLabelPool*)((R5LabelRPCNetView*)pNetGame)->pools->pLabelPool;
 if(pool) {
  WORD id; DWORD color;
  float x,y,z,distance;
  bool testLOS=false;
  WORD playerId,vehicleId;
  char text[2049]; memset(text,0,sizeof(text));
  stream.Read(id);stream.Read(color);
  stream.Read(x);stream.Read(y);stream.Read(z);stream.Read(distance);
  stream.Read((BYTE&)testLOS);stream.Read(playerId);stream.Read(vehicleId);
  StringCompressor::Instance()->DecodeString(text,2048,&stream,0);
  if(id<MAX_LABELS)pool->New(id,text,color,x,y,z,distance,testLOS,playerId,vehicleId);
 }
}
