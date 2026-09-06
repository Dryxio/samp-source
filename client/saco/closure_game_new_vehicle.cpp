// R5 A0250: actual vehicle factory. Adapted from the 0.2.5 NewVehicle.
#include "main.h"
// Pointer-only view of existing CGame state. Never allocate this view.
class R5GameVehicleCreationView {
 BYTE prefix[0x6e];
 BYTE keepLoadedVehicles[212];
public:
 CVehicle *NewVehicle(int model,float x,float y,float z,float rotation,int creationFlag);
};
typedef char R5CompleteAllocatedVehicle[sizeof(CVehicle)==0xb8?1:-1];
CVehicle *R5GameVehicleCreationView::NewVehicle(int model,float x,float y,float z,float rotation,int creationFlag)
{
 BOOL keepModel=FALSE;
 if(keepLoadedVehicles[model-400])keepModel=TRUE;
 CVehicle *vehicle=new CVehicle(model,x,y,z,rotation,keepModel,creationFlag);
 // Original checks the native handle after construction, without deleting a
 // failed native vehicle or adding a separate null-allocation guard.
 return vehicle->m_pVehicle ? vehicle : NULL;
}
