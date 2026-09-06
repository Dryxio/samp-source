#include "main.h"
#include <stddef.h>
#pragma pack(push,1)
struct R5RemoteDistanceQueryView {
 BYTE unknown0[0x10a]; BYTE state;
 BYTE unknown10B[0xd2]; CPlayerPed *ped; CVehicle *vehicle;
 float GetDistanceFromLocalPlayer();
};
#pragma pack(pop)
typedef char DistancePedOffset[(offsetof(R5RemoteDistanceQueryView,ped)==0x1dd)?1:-1];
float R5RemoteDistanceQueryView::GetDistanceFromLocalPlayer()
{
 if(!ped)return 10000.0f;
 if(state==19 && vehicle)return vehicle->GetDistanceFromLocalPlayerPed();
 return ped->GetDistanceFromLocalPlayerPed();
}
