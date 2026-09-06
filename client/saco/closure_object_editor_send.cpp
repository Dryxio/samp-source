// Object editor26EB60, original allocation117 and constructor71B60 audited separately.
// Partial pointer view only, no instances or new allocation claims.
#include "main.h"
extern CNetGame *pNetGame;
extern const int r5RpcEditObject;
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

struct R5EditorObjectPoolView {
    int lastObjectId;
    BOOL active[1000];
    CObject *objects[1000];
    CObject *GetAt(WORD id) {
        if(id>1000) return NULL;
        if(!active[id]) return NULL;
        return objects[id];
    }
};
struct R5EditorPoolsView { BYTE unknown[0xc]; R5EditorObjectPoolView *objects; };
struct R5EditorNetView { BYTE unknown[0x3de]; R5EditorPoolsView *pools; };
//728B0/513. Preserve original inclusive comparison id>1000, no corrected bounds.
void R5ObjectEditorView::SendObjectEdit(int response)
{
    R5EditorObjectPoolView *pool=((R5EditorNetView*)pNetGame)->pools->objects;
    CObject *object=pool->GetAt(objectId);
    if(object) {
        if(response==2 && (int)(GetTickCount()-lastUpdate)<250) return;
        lastUpdate=GetTickCount();
        float angleX=rotation.X;
        float angleY=rotation.Y;
        float angleZ=rotation.Z;
        MATRIX4X4 matrix;
        ((CEntity*)object)->GetMatrix(&matrix);
        RakNet::BitStream stream;
        if(playerObject) stream.Write1();
        else stream.Write0();
        stream.Write<WORD>(objectId);
        stream.Write(response);
        stream.Write(matrix.pos.X);
        stream.Write(matrix.pos.Y);
        stream.Write(matrix.pos.Z);
        stream.Write(angleX);
        stream.Write(angleY);
        stream.Write(angleZ);
        pNetGame->GetRakClient()->RPC((char*)&r5RpcEditObject,&stream,HIGH_PRIORITY,RELIABLE_ORDERED,0,FALSE);
    }
}
