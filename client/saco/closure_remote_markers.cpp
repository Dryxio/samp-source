#include "main.h"
extern CGame *pGame;
#pragma pack(push,1)
// Pointer-only remote-player observation, never constructed or allocated.
struct R5RemoteMarkerView {
 BYTE prefix[0x1dd];
 CPlayerPed *ped;
 CVehicle *vehicle;
 PLAYERID playerId;
 VEHICLEID vehicleId;
 BOOL markerActive;
 int markerX,markerY,markerZ;
 DWORD marker;
 void SetMarkerActive(BOOL active);
 void ShowMarker(short x,short y,short z);
};
#pragma pack(pop)
void R5RemoteMarkerView::SetMarkerActive(BOOL active)
{
 if(!active && marker) {
  pGame->DisableMarker(marker);
  marker=active;
 }
 markerActive=active;
}
void R5RemoteMarkerView::ShowMarker(short x,short y,short z)
{
 markerActive=TRUE;
 if(marker) {
  pGame->DisableMarker(marker);
  marker=0;
 }
 if(!ped) {
  marker=pGame->CreateRadarMarkerIcon(0,(float)x,(float)y,(float)z,playerId,0);
  markerX=x;
  markerY=y;
  markerZ=z;
 }
}
