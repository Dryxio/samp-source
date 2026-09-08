// R5 object editor entry operations from complete reference handlers.
#include "main.h"
extern CNetGame *pNetGame;
#include <stddef.h>
extern CGame *pGame;
class R5ObjectEditorView;
extern R5ObjectEditorView *r5ObjectEditor;
class R5ObjectSelectionView {
 BOOL enabled; WORD selectedObjectId;
public: R5ObjectSelectionView(); void Toggle(BOOL enable);
};
extern R5ObjectSelectionView *r5ObjectSelection;
struct R5ObjectSelectionEnabledView { BOOL enabled; };
#pragma pack(push,1)
struct R5InputRestoreCountdownView { BYTE unknown[0x65]; int countdown; };
class R5ObjectEditorBeginView {
 BYTE unknown0[0x78];
 int mode; int action; BOOL enabled; BYTE unknown84[4];
 WORD objectId; int attachedSlot; BOOL playerObject; VECTOR rotation;
 int lastUpdate; BYTE attached;
public:
 void SetAction(int value);
 void BeginAttached(int slot);
};
#pragma pack(pop)
void R5ObjectEditorBeginView::SetAction(int value) {
 if(mode==1 && value==2)value=0;
 action=value;
}
void R5ObjectEditorBeginView::BeginAttached(int slot) {
 enabled=TRUE;
 mode=2;
 SetAction(0);
 attachedSlot=slot;
 objectId=0xffff;
 attached=1;
 pGame->ToggleKeyInputsDisabled(0,TRUE);
 ((R5InputRestoreCountdownView*)pGame)->countdown=0;
 pGame->ProcessInputDisabling();
}
void EditAttachedObjectRPCTransfer(RPCParameters *rpcParams) {
 PCHAR data=(PCHAR)rpcParams->input;
 int bits=rpcParams->numberOfBitsOfData;
 RakNet::BitStream stream(data,bits/8+1,false);
 int slot=0;
 stream.Read(slot);
 if(r5ObjectSelection && ((R5ObjectSelectionEnabledView*)r5ObjectSelection)->enabled)r5ObjectSelection->Toggle(FALSE);
 ((R5ObjectEditorBeginView*)r5ObjectEditor)->BeginAttached(slot);
}

class R5TextDrawSelectorView {
 BOOL enabled; DWORD highlightColor; WORD selectedId;
public: void ClearAll(); void UpdateHover(); void Enable(DWORD color);
};
extern R5TextDrawSelectorView *r5TextdrawSelector;
class R5TextDrawClickView {
 BOOL enabled; DWORD highlightColor; WORD selectedId;
public: void SendClick(); void Disable(); BOOL MsgProc(UINT message,WPARAM wParam,LPARAM lParam);
};

#pragma pack(push,1)
struct R5EditObjectRotationView { BYTE gap[0xad]; VECTOR rotation; };
struct R5EditObjectPoolView {
 int lastId; BOOL slots[1000]; R5EditObjectRotationView *objects[1000];
 R5EditObjectRotationView *GetAt(WORD id) { if(id>1000)return NULL; if(slots[id])return objects[id]; return NULL; }
};
struct R5EditObjectNetView { BYTE gap[0x3de]; NETGAME_POOLS *pools; };
class R5ObjectEditorWorldBeginView {
 BYTE unknown0[0x78]; int mode; int action; BOOL enabled; BYTE unknown84[4];
 WORD objectId; int attachedSlot; BOOL playerObject; VECTOR rotation;
 int lastUpdate; BYTE attached;
public: void BeginObject(WORD id,BOOL isPlayerObject);
};

#pragma pack(pop)
void R5ObjectEditorWorldBeginView::BeginObject(WORD id,BOOL isPlayerObject) {
 enabled=TRUE;
 mode=1;
 ((R5ObjectEditorBeginView*)this)->SetAction(0);
 objectId=id;
 playerObject=isPlayerObject;
 attached=0;
 pGame->ToggleKeyInputsDisabled(0,TRUE);
 ((R5InputRestoreCountdownView*)pGame)->countdown=0;
 pGame->ProcessInputDisabling();
 if(pNetGame) {
  R5EditObjectPoolView *pool=(R5EditObjectPoolView*)((R5EditObjectNetView*)pNetGame)->pools->pObjectPool;
  R5EditObjectRotationView *object=pool->GetAt(id);
  if(object) { rotation.X=object->rotation.X; rotation.Y=object->rotation.Y; rotation.Z=object->rotation.Z; }
 }
}
void EditObjectRPCTransfer(RPCParameters *rpcParams) {
 PCHAR data=(PCHAR)rpcParams->input;
 int bits=rpcParams->numberOfBitsOfData;
 RakNet::BitStream stream(data,bits/8+1,false);
 bool isPlayerObject=false;
 int id=0xffff;
 stream.Read(isPlayerObject);
 stream.Read((WORD&)id);
 if(r5ObjectSelection && ((R5ObjectSelectionEnabledView*)r5ObjectSelection)->enabled)r5ObjectSelection->Toggle(FALSE);
 if(r5TextdrawSelector && ((R5ObjectSelectionEnabledView*)r5TextdrawSelector)->enabled)((R5TextDrawClickView*)r5TextdrawSelector)->Disable();
 if(isPlayerObject)((R5ObjectEditorWorldBeginView*)r5ObjectEditor)->BeginObject(id,TRUE);
 else ((R5ObjectEditorWorldBeginView*)r5ObjectEditor)->BeginObject(id,FALSE);
}
