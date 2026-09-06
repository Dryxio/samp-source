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
void R5CompleteLabelPool::New(WORD id,char *text,DWORD color,float x,float y,float z,float distance,bool testLOS,WORD playerId,WORD vehicleId) {
 if(id>=MAX_LABELS) return;
 if(slots[id]) {
  if(labels[id].text) free(labels[id].text);
  memset(&labels[id],0,sizeof(R5CompleteLabelRecord));
 }
 color=(color<<24)|(color>>8);
 labels[id].text=(char*)calloc(1,strlen(text)+1);
 strcpy(labels[id].text,text);
 if(strlen(labels[id].text)<400) unnamed_100B6100(labels[id].text,399);
 labels[id].color=color;
 labels[id].position.X=x;
 labels[id].position.Y=y;
 labels[id].position.Z=z;
 labels[id].drawDistance=distance;
 labels[id].testLOS=testLOS;
 labels[id].playerId=playerId;
 labels[id].vehicleId=vehicleId;
 slots[id]=TRUE;
}
