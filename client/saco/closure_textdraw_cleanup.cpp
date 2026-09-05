// R5 cleanup candidate; textdraw layout is only a verified field view, never allocated here.
#include <windows.h>
#define MAX_TEXT_DRAWS 2304
// Partial field view: unknown bytes are not reconstructed layout or data coverage.
// No construction, allocation or pointer arithmetic on this type is supported.
#include <stddef.h>
class R5TextDrawCleanupView { unsigned char unknown_prefix[0x9a3]; int textureSlot; public: ~R5TextDrawCleanupView(); };
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
    ~R5TextDrawPool();
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
