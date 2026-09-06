// R5 actor RPC reconstruction. Reuses 0.2.5 animation serialization only.
#include "main.h"
extern CNetGame *pNetGame;
class RPCActorPoolView {
 int count;
 CActorPed *actors[MAX_ACTORS];
 BOOL slots[MAX_ACTORS];
public:
 CActorPed *GetAt(WORD id) { if(id>=MAX_ACTORS || !slots[id])return NULL; return actors[id]; }
};
struct RPCActorNetGameView { BYTE unknown[0x3de]; NETGAME_POOLS *pools; };
static inline CActorPool *RPCActorPool() { return ((RPCActorNetGameView*)pNetGame)->pools->pActorPool; }
void DestroyActorRPCTransfer(RPCParameters *rpcParams)
{
 PCHAR Data=(PCHAR)rpcParams->input;
 int bitLength=rpcParams->numberOfBitsOfData;
 RakNet::BitStream bsData(Data,bitLength/8+1,false);
 CActorPool *pool=RPCActorPool();
 if(!pool)return;
  WORD id;
  bsData.Read(id);
  pool->Delete(id);
}
void ClearActorAnimationRPCTransfer(RPCParameters *rpcParams)
{
 PCHAR Data=(PCHAR)rpcParams->input;
 int bitLength=rpcParams->numberOfBitsOfData;
 RakNet::BitStream bsData(Data,bitLength/8+1,false);
 WORD id;
 bsData.Read(id);
 RPCActorPoolView *pool=(RPCActorPoolView*)RPCActorPool();
 if(pool) {
  CActorPed *actor=pool->GetAt(id);
  if(actor)actor->ClearAnimations();
 }
}
void SetActorRotationRPCTransfer(RPCParameters *rpcParams)
{
 PCHAR Data=(PCHAR)rpcParams->input;
 int bitLength=rpcParams->numberOfBitsOfData;
 RakNet::BitStream bsData(Data,bitLength/8+1,false);
 WORD id; float rotation;
 bsData.Read(id);bsData.Read(rotation);
 RPCActorPoolView *pool=(RPCActorPoolView*)RPCActorPool();
 if(pool) {
  CActorPed *actor=pool->GetAt(id);
  if(actor)actor->SetTargetRotation(rotation);
 }
}
void SetActorPositionRPCTransfer(RPCParameters *rpcParams)
{
 PCHAR Data=(PCHAR)rpcParams->input;
 int bitLength=rpcParams->numberOfBitsOfData;
 RakNet::BitStream bsData(Data,bitLength/8+1,false);
 WORD id; VECTOR pos;
 bsData.Read(id);bsData.Read(pos.X);bsData.Read(pos.Y);bsData.Read(pos.Z);
 RPCActorPoolView *pool=(RPCActorPoolView*)RPCActorPool();
 if(pool) {
  CActorPed *actor=pool->GetAt(id);
  if(actor)actor->TeleportTo(pos.X,pos.Y,pos.Z);
 }
}
void SetActorHealthRPCTransfer(RPCParameters *rpcParams)
{
 PCHAR Data=(PCHAR)rpcParams->input;
 int bitLength=rpcParams->numberOfBitsOfData;
 RakNet::BitStream bsData(Data,bitLength/8+1,false);
 WORD id; float health;
 bsData.Read(id);bsData.Read(health);
 RPCActorPoolView *pool=(RPCActorPoolView*)RPCActorPool();
 if(pool) {
  CActorPed *actor=pool->GetAt(id);
  if(actor)actor->SetHealth(health);
 }
}
void ApplyActorAnimationRPCTransfer(RPCParameters *rpcParams)
{
 PCHAR Data=(PCHAR)rpcParams->input;
 int bitLength=rpcParams->numberOfBitsOfData;
 RakNet::BitStream bsData(Data,bitLength/8+1,false);
 RPCActorPoolView *pool=(RPCActorPoolView*)RPCActorPool();
 if(pool) {
  WORD id;
  BYTE byteAnimLibLen,byteAnimNameLen;
  char szAnimLib[256];
  char szAnimName[256];
  float fS;
  bool opt1,opt2,opt3,opt4;
  int opt5;
  memset(szAnimLib,0,256);memset(szAnimName,0,256);
  bsData.Read(id);
  bsData.Read(byteAnimLibLen);bsData.Read(szAnimLib,byteAnimLibLen);
  bsData.Read(byteAnimNameLen);bsData.Read(szAnimName,byteAnimNameLen);
  bsData.Read(fS);bsData.Read(opt1);bsData.Read(opt2);bsData.Read(opt3);bsData.Read(opt4);bsData.Read(opt5);
  szAnimLib[byteAnimLibLen]=0;szAnimName[byteAnimNameLen]=0;
  CActorPed *actor=pool->GetAt(id);
  if(actor)actor->ApplyAnimation(szAnimName,szAnimLib,fS,(int)opt1,(int)opt2,(int)opt3,(int)opt4,opt5);
 }
}
