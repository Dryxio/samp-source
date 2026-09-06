#include "main.h"
extern CGame *pGame;
extern CCmdWindow *pCmdWindow;
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

//72BC0/109; requires both real send providers before acceptance.
void R5ObjectEditorView::Close(BOOL accept)
{
    int response=0;
    if(accept) response=1;
    enabled=FALSE;
    pGame->ToggleKeyInputsDisabled(0,TRUE);
    if(pCmdWindow->m_bEnabled) {
        pCmdWindow->Disable();
        pCmdWindow->Enable();
    }
    if(mode==2) SendAttachedEdit(response);
    if(mode==1) SendObjectEdit(response);
}
