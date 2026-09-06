//1D650/247, independent protocol source from R5. Closure pending object-editor Close chain.
#include "main.h"
class R5ObjectSelectionView {
    BOOL enabled;
    WORD selectedObjectId;
public:
    R5ObjectSelectionView();
    void Toggle(BOOL enable);
};
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
class R5TextDrawSelectorView {
    BOOL enabled;
    DWORD highlightColor;
    WORD selectedId;
public:
    void ClearAll();
    void UpdateHover();
    void Enable(DWORD color);
};
class R5TextDrawClickView {
    BOOL enabled;
    DWORD highlightColor;
    WORD selectedId;
public:
    void SendClick();
    void Disable();
    BOOL MsgProc(UINT message,WPARAM wParam,LPARAM lParam);
};

extern R5ObjectSelectionView *r5ObjectSelection;
extern R5ObjectEditorView *r5ObjectEditor;
extern R5TextDrawSelectorView *r5TextdrawSelector;
struct R5EditorEnabledView { BYTE unknown[0x80]; BOOL enabled; };
void ScrSelectTextDrawTransfer(RPCParameters *rpcParams)
{
    PCHAR data=reinterpret_cast<PCHAR>(rpcParams->input);
    int bitLength=rpcParams->numberOfBitsOfData;
    RakNet::BitStream stream(data,(bitLength/8)+1,false);
    bool enable=false;
    DWORD color=0;
    stream.Read(enable);
    stream.Read(color);
    if(r5ObjectSelection && *(BOOL*)r5ObjectSelection) r5ObjectSelection->Toggle(FALSE);
    if(r5ObjectEditor && ((R5EditorEnabledView*)r5ObjectEditor)->enabled) r5ObjectEditor->Close(FALSE);
    if(enable) {
        if(r5TextdrawSelector) r5TextdrawSelector->Enable(color);
    } else {
        if(r5TextdrawSelector) ((R5TextDrawClickView*)r5TextdrawSelector)->Disable();
    }
}
