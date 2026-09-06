#include "main.h"
#include <stddef.h>
#pragma pack(push,1)
struct R5ImeCandidateListState
{
 WCHAR awszCandidate[10][256];
 CUniBuffer HoriCand;
 int nFirstSelected;
 int nHoriSelectedLen;
 DWORD dwCount;
 DWORD dwSelection;
 DWORD dwPageSize;
 int nReadingError;
 bool bShowWindow;
 RECT rcCandidate;
};
#pragma pack(pop)
extern R5ImeCandidateListState r5ImeCandidateState;
extern DWORD dwImeWaitTick;
// Import-only alias. The real single-byte owner remains the accepted IME static.
extern "C" bool r5ImeShowReadingWindowImport;
#pragma comment(linker,"/alternatename:_r5ImeShowReadingWindowImport=?s_bShowReadingWindow@CDXUTIMEEditBox@@1_NA")
class R5ImeInteractionView { public: static bool IsActive(); };
class R5ChatInputQueryView {
public:
 IDirect3DDevice9 *device;
 CDXUTDialog *dialog;
 CDXUTIMEEditBox *edit;
 bool IsImeInteractionActive();
};
bool R5ImeInteractionView::IsActive()
{
 if(r5ImeCandidateState.bShowWindow) return true;
 if(r5ImeShowReadingWindowImport) return true;
 if((int)(GetTickCount()-dwImeWaitTick)<300) return true;
 return false;
}
bool R5ChatInputQueryView::IsImeInteractionActive()
{
 if(edit) return R5ImeInteractionView::IsActive();
 return false;
}
