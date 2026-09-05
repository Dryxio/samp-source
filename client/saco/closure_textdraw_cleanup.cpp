// R5 cleanup candidate; textdraw layout is only a verified field view, never allocated here.
#include <windows.h>
#define MAX_TEXT_DRAWS 2304
// Partial field view: unknown bytes are not reconstructed layout or data coverage.
// No construction, allocation or pointer arithmetic on this type is supported.
#include <stddef.h>
#include <string.h>
class R5TextDrawCleanupView {
    unsigned char unknown_prefix[0x987];
    DWORD style;
    unsigned char unknown_between[0x18];
    int textureSlot;
public:
    ~R5TextDrawCleanupView();
    void ReleasePreviewTexture();
};
void DestroyTexture(DWORD texture);
extern DWORD r5TextdrawTextures[200];
extern BOOL r5TextdrawTextureUsed[200];
void ReleaseTextdrawTexture(int slot)
{
    if(slot >= 0 && slot < 200) {
        r5TextdrawTextureUsed[slot] = FALSE;
        if(r5TextdrawTextures[slot]) DestroyTexture(r5TextdrawTextures[slot]);
        r5TextdrawTextures[slot] = 0;
    }
}
R5TextDrawCleanupView::~R5TextDrawCleanupView() { ReleaseTextdrawTexture(textureSlot); }

class R5TextDrawPool {
    BOOL m_bSlotState[MAX_TEXT_DRAWS];
    R5TextDrawCleanupView* m_pTextDraw[MAX_TEXT_DRAWS];
public:
    R5TextDrawPool();
    ~R5TextDrawPool();
    void ReleasePreviewTextures();
    void Delete(WORD wText);
};
R5TextDrawPool::~R5TextDrawPool()
{
	int x=0;
	while(x != MAX_TEXT_DRAWS) {
		if(m_pTextDraw[x]) {
			delete m_pTextDraw[x];
			m_pTextDraw[x] = NULL;
			m_bSlotState[x] = FALSE;
		}
		x++;
	}
}
void R5TextDrawPool::Delete(WORD wText)
{
	if(m_pTextDraw[wText]) {
		delete m_pTextDraw[wText];
		m_pTextDraw[wText] = NULL;
		m_bSlotState[wText] = FALSE;
	}	
}

// Proposed original B36B0 (44); style5 preview texture release.
void R5TextDrawCleanupView::ReleasePreviewTexture()
{
    if(style == 5) {
        if(textureSlot != -1) {
            ReleaseTextdrawTexture(textureSlot);
            textureSlot = -1;
        }
    }
}
// Proposed original 1E8A0 (36); all active pool entries.
void R5TextDrawPool::ReleasePreviewTextures()
{
    int x=0;
    while(x != MAX_TEXT_DRAWS) {
        if(m_bSlotState[x]) m_pTextDraw[x]->ReleasePreviewTexture();
        x++;
    }
}
// Base game/unnamed_1.cpp translated only to accepted descriptive store identities.
BOOL InitializeTextdrawTextures()
{
    memset(r5TextdrawTextureUsed, 0, sizeof(BOOL)*200);
    memset(r5TextdrawTextures, 0, sizeof(DWORD)*200);
    return TRUE;
}
int AllocateTextdrawTexture()
{
    int x=0;
    while(x!=200) {
        if(!r5TextdrawTextureUsed[x]) {
            r5TextdrawTextureUsed[x]=TRUE;
            return x;
        }
        x++;
    }
    return -1;
}
