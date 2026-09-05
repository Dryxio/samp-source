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

class R5TextDrawPool {
    BOOL m_bSlotState[MAX_TEXT_DRAWS];
    R5TextDrawCleanupView* m_pTextDraw[MAX_TEXT_DRAWS];
public:
    R5TextDrawPool();
    ~R5TextDrawPool();
    void ReleasePreviewTextures();
    void Delete(WORD wText);
};
BOOL InitializeTextdrawTextures();
R5TextDrawPool::R5TextDrawPool()
{
    int x=0;
    while(x!=MAX_TEXT_DRAWS) {
        m_pTextDraw[x]=0;
        m_bSlotState[x]=0;
        x++;
    }
    InitializeTextdrawTextures();
}
