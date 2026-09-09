// Existing R5 object-pool storage; pointer-only view, never allocated here.
#include "main.h"
#include <stddef.h>
#pragma pack(push,1)
class R5StatisticsObjectPoolView
{
public:
 int lastObjectId;
 BOOL slots[1000];
 CObject *objects[1000];
 int GetCount();
};
#pragma pack(pop)
typedef char R5StatsObjectSlots[(offsetof(R5StatisticsObjectPoolView,slots)==4)?1:-1];
typedef char R5StatsObjectArray[(offsetof(R5StatisticsObjectPoolView,objects)==4004)?1:-1];
int R5StatisticsObjectPoolView::GetCount()
{
 int count=0;
 int index=0;
 while(index!=1000)
 {
  if(slots[index]) ++count;
  ++index;
 }
 return count;
}
