#include "main.h"
struct R5VehicleTrailerPoolView {
 BYTE unknown0[0x1134]; CVehicle *vehicles[2000]; BOOL active[2000];
 __declspec(dllexport) CVehicle *GetAt(WORD id);
 __declspec(dllexport) BOOL GetSlotState(WORD id);
};
CVehicle *R5VehicleTrailerPoolView::GetAt(WORD id) {
 if(id>=2000)return NULL;
 if(active[id])return vehicles[id];
 return NULL;
}
BOOL R5VehicleTrailerPoolView::GetSlotState(WORD id) {
 if(id>=2000)return FALSE;
 return active[id];
}
