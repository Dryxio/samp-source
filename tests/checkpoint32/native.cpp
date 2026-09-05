// Calls the actual linked C++ methods against synthetic GTA memory.
#include "../../client/saco/main.h"
#include "../../client/saco/game/util.h"
#include <stddef.h>
#include <stdio.h>
extern bool IsNumeric(char*);
static unsigned checks;
#define CHECK(x) do { ++checks; if(!(x)) {printf("FAIL line %u\n",__LINE__);return 1;} } while(0)
int main() {
 CHECK(sizeof(CEntity)==0x48); CHECK(sizeof(CActorPed)==0x56);
 CHECK(offsetof(CEntity,m_pEntity)==0x40); CHECK(offsetof(CEntity,m_dwGTAId)==0x44);
 CHECK(offsetof(CActorPed,m_pPed)==0x48); CHECK(offsetof(PED_TYPE,fHealth)==0x540);
 CHECK(offsetof(PED_TYPE,fArmour)==0x548); CHECK(offsetof(PED_TYPE,dwAction)==0x530);
 unsigned char actorMemory[0x56]={0},pedMemory[0x800]={0};
 CActorPed* actor=(CActorPed*)actorMemory; PED_TYPE* ped=(PED_TYPE*)pedMemory;
 CHECK(actor->GetHealth()==0); CHECK(actor->GetArmour()==0);
 CHECK(actor->GetStateFlags()==0); CHECK(actor->IsDead()); CHECK(!actor->IsInVehicle());
 actor->SetArmour(17);actor->SetStateFlags(0xFFFFFFFF);actor->SetActionTrigger(255);
 actor->m_dwGTAId=123; actor->Destroy();
 CHECK(!actor->m_pPed && !actor->m_pEntity && actor->m_dwGTAId==0);
 actor->m_pPed=ped; actor->m_pEntity=(ENTITY_TYPE*)ped;
 for(unsigned i=0;i<1024;++i) {
  DWORD flags=i*0x1020304U;
  actor->SetStateFlags(flags);CHECK(actor->GetStateFlags()==flags);
  CHECK(actor->IsInVehicle()==((flags>>8)&1));
  actor->SetActionTrigger((BYTE)i);CHECK(actor->GetActionTrigger()==(BYTE)i);
  CHECK(ped->dwAction==(BYTE)i);
  float health=(int)i-512.0f;ped->fHealth=health;
  CHECK(actor->GetHealth()==health);CHECK(actor->IsDead()==!(health>0));
  actor->SetArmour(health);CHECK(actor->GetArmour()==health);
  ENTITY_TYPE* entity=(ENTITY_TYPE*)ped;
  entity->vtable=0;entity->dwProcessingFlags=flags;
  actor->SetCollisionChecking(0);CHECK(entity->dwProcessingFlags==(flags&~1U));CHECK(!actor->IsCollisionCheckingEnabled());
  actor->SetCollisionChecking(-1);CHECK(entity->dwProcessingFlags==(flags|1U));CHECK(actor->IsCollisionCheckingEnabled());
  entity->dwProcessingFlags=flags;actor->SetGravityProcessing(1);CHECK(entity->dwProcessingFlags==(flags&0x7FFFFFFD));
  actor->SetGravityProcessing(0);CHECK(entity->dwProcessingFlags==(flags|0x80000002));
  entity->dwProcessingFlags=flags;actor->DisableStreaming();CHECK(entity->dwProcessingFlags==(flags|0x80400));
  actor->EnableTunnelTransition();CHECK(entity->dwProcessingFlags==(flags|0x80080400));
  entity->dwPhysFlags=flags;actor->MakeNonCollidable();CHECK(entity->dwPhysFlags==(flags&~8U));
  VECTOR v={(float)i,-(float)i,0.25f*i},got;
  actor->SetMoveSpeedVector(v);actor->GetMoveSpeedVector(&got);
  CHECK(!memcmp(&v,&got,sizeof(v)));
  actor->SetTurnSpeedVector(v);actor->GetTurnSpeedVector(&got);
  CHECK(!memcmp(&v,&got,sizeof(v)));
 }
 MATRIX4X4 matrix,input,output;
 memset(&matrix,0x12,sizeof(matrix));memset(&output,0x34,sizeof(output));
 for(unsigned j=0;j<16;++j)((float*)&input)[j]=(float)(j+1);
 ped->entity.mat=&matrix;actor->SetMatrix(input);actor->GetMatrix(&output);
 for(j=0;j<16;++j) {
  if((j%4)!=3) {CHECK(((float*)&matrix)[j]==((float*)&input)[j]);CHECK(((float*)&output)[j]==((float*)&input)[j]);}
  else {CHECK(((DWORD*)&matrix)[j]==0x12121212);CHECK(((DWORD*)&output)[j]==0x34343434);}
 }
 CHECK(FloatDifference(8,3)==5);CHECK(FloatOffset(3,8)==5);CHECK(FloatOffset(8,3)==5);
 CHECK(SquaredDistanceBetweenHorizontalPoints(0,0,3,4)==25);
 CHECK(DistanceBetweenHorizontalPoints(0,0,3,4)==5);CHECK(DistanceBetweenPoints(0,0,0,2,3,6)==7);
 CHECK(IsNumeric(""));CHECK(IsNumeric("0123456789"));CHECK(!IsNumeric("-1"));CHECK(!IsNumeric("12.3"));
 actor->m_pEntity=0;CHECK(!actor->IsAdded());actor->CEntity::Remove();
 CHECK(sizeof(CActorPool)==0x4E24);
 unsigned char poolMemory[sizeof(CActorPool)];memset(poolMemory,0xCC,sizeof(poolMemory));
 CActorPool* pool=new(poolMemory) CActorPool;
 DWORD* words=(DWORD*)poolMemory;
 CHECK(words[0]==0);
 for(unsigned k=0;k<1000;++k) {CHECK(words[1+k]==0);CHECK(words[0x3E9+k]==0);CHECK(words[0x7D1+k]==0);}
 for(k=0x2EE4;k<sizeof(poolMemory);++k) CHECK(poolMemory[k]==0xCC);
 CHECK(pool->FindIDFromGtaPtr(0)==0);CHECK(pool->FindIDFromGtaPtr(1)==0xFFFF);
 CHECK(!pool->Delete(1000));CHECK(!pool->Delete(65535));CHECK(!pool->Delete(0));
 words[0x3E9+999]=1;words[0x7D1+999]=0x4567;pool->UpdateCount();CHECK(words[0]==999);
 CHECK(pool->FindIDFromGtaPtr(0x4567)==999);CHECK(!pool->Delete(999));
 pool->DeleteAll();CHECK(words[0x3E9+999]==1); // a populated slot without an actor is left intact
 words[0x3E9+999]=0;pool->UpdateCount();CHECK(words[0]==0);
 printf("PASS %u native checks at %p\n",checks,GetModuleHandleA("actor.dll"));return 0;
}
