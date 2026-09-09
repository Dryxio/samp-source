// R5 complete CPlayerPed pointer-only view. Actual attachment owner is CObject;
// its CEntity base starts at zero and owns the real virtual deleting destructor.
#include "main.h"
#include <stddef.h>
#pragma pack(push,1)
class R5AttachedObjectsPedView
{
public:
 BYTE prefix[0x4c];
 BOOL active[10];
 struc_97 info[10];
 CEntity *objects[10];
 BYTE tail[sizeof(CPlayerPed)-0x2a4];
 void RemoveAttachedObject(int index);
 BOOL GetAttachedObjectSlotState(int index);
};
#pragma pack(pop)
typedef char R5AttachedWholeSize[(sizeof(R5AttachedObjectsPedView)==sizeof(CPlayerPed))?1:-1];
typedef char R5AttachedStateOffset[(offsetof(R5AttachedObjectsPedView,active)==offsetof(CPlayerPed,field_4C))?1:-1];
typedef char R5AttachedInfoOffset[(offsetof(R5AttachedObjectsPedView,info)==offsetof(CPlayerPed,field_74))?1:-1];
typedef char R5AttachedObjectOffset[(offsetof(R5AttachedObjectsPedView,objects)==offsetof(CPlayerPed,field_27C))?1:-1];
void R5AttachedObjectsPedView::RemoveAttachedObject(int index)
{
 if(index<0 || index>=10) return;
 if(active[index]!=TRUE) return;
 if(objects[index])
 {
  delete objects[index];
  objects[index]=NULL;
 }
 memset(&info[index],0,sizeof(info[index]));
 active[index]=FALSE;
}
BOOL R5AttachedObjectsPedView::GetAttachedObjectSlotState(int index)
{
 if(index<0 || index>=10) return FALSE;
 return active[index]==TRUE;
}
