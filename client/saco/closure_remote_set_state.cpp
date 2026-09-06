#include "main.h"
#pragma pack(push,1)
// Complete observed R5 driver-sync record. Bytes not interpreted by these
// functions remain explicit protocol bytes, not invented layout padding.
struct R5RemoteDriverSync {
 VEHICLEID vehicleId;
 BYTE otherControlsAndMovement[50];
 BYTE health;
 BYTE armour;
 BYTE weaponAndFlags;
 BYTE vehicleExtras[8];
};
struct R5RemoteDriverUpdateView {
 BYTE unknown0[0xc];
 BYTE afk;
 BYTE unknownD[0xc];
 R5RemoteDriverSync driverSync;
 BYTE unknown58[0xb2];
 BYTE state;
 BYTE seat;
 BYTE unknown10C[0xa0];
 float reportedArmour;
 float reportedHealth;
 BYTE unknown1B4[4];
 BYTE updateType;
 DWORD lastReceivedTick;
 DWORD lastDriverTimestamp;
 BYTE unknown1C1[0x1c];
 CPlayerPed *ped;
 CVehicle *vehicle;
 BYTE unknown1E5[2];
 VEHICLEID vehicleId;
 void StateChange(BYTE newState,BYTE oldState);
 void StoreDriverSync(R5RemoteDriverSync *sync,DWORD timestamp);
};
#pragma pack(pop)

// Pointer-only derived observation; no new fields, virtuals or allocations.
struct R5RemoteStateSetterView : R5RemoteDriverUpdateView {
 void SetState(BYTE newState);
};
void R5RemoteStateSetterView::SetState(BYTE newState)
{
 if(newState!=state) {
  StateChange(newState,state);
  state=newState;
 }
}
