//72AC0/241. Candidate only: actual FindPlayerPed1010/94 remains a dependency.
#include "main.h"
extern CGame *pGame;
extern CNetGame *pNetGame;
extern const int r5RpcEditAttachedObject;
class R5ObjectEditorView {
    BYTE unknown0[0x78];
    int mode;
    BYTE unknown7c[4];
    BOOL enabled;
    BYTE unknown84[4];
    WORD objectId;
    int attachedSlot;
    BOOL playerObject;
    VECTOR rotation;
    int lastUpdate;
public:
    void SendObjectEdit(int response);
    void SendAttachedEdit(int response);
    void Close(BOOL accept);
};

static __forceinline struc_97 *GetAttachedInfo(CPlayerPed *ped,int slot)
{
    if(slot<0 || slot>=10) return NULL;
    if(!ped->field_4C[slot]) return NULL;
    return &ped->field_74[slot];
}
void R5ObjectEditorView::SendAttachedEdit(int response)
{
    RakNet::BitStream stream;
    int slot=attachedSlot;
    struc_97 *info=GetAttachedInfo(pGame->FindPlayerPed(),slot);
    stream.Write(response);
    stream.Write(attachedSlot);
    stream.Write((const char*)info,52);
    pNetGame->GetRakClient()->RPC((char*)&r5RpcEditAttachedObject,&stream,HIGH_PRIORITY,RELIABLE_ORDERED,0,FALSE);
}
