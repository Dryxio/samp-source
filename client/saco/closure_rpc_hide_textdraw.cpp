// 0.2.5 ScrHideTextDraw transfer, removing debug output absent from R5.
#include "main.h"
extern CNetGame *pNetGame;
class R5TextDrawCleanupView {
    unsigned char unknown_prefix[0x987];
    DWORD style;
    unsigned char unknown_between[0x18];
    int textureSlot;
public:
    ~R5TextDrawCleanupView();
    void ReleasePreviewTexture();
};
class R5TextDrawPool {
    BOOL m_bSlotState[MAX_TEXT_DRAWS];
    R5TextDrawCleanupView* m_pTextDraw[MAX_TEXT_DRAWS];
public:
    R5TextDrawPool();
    ~R5TextDrawPool();
    void ReleasePreviewTextures();
    void Delete(WORD wText);
};

struct R5RpcTextdrawPoolsView { BYTE unknown[0x1c]; R5TextDrawPool *textdraw; };
struct R5RpcTextdrawNetView { BYTE unknown[0x3de]; R5RpcTextdrawPoolsView *pools; };
void ScrHideTextDrawTransfer(RPCParameters *rpcParams)
{
    PCHAR Data=reinterpret_cast<PCHAR>(rpcParams->input);
    int iBitLength=rpcParams->numberOfBitsOfData;
    PlayerID sender=rpcParams->sender;
    RakNet::BitStream bsData(Data,(iBitLength/8)+1,false);
    R5TextDrawPool *pool=((R5RpcTextdrawNetView*)pNetGame)->pools->textdraw;
    if(pool) {
        WORD id;
        bsData.Read(id);
        pool->Delete(id);
    }
}
