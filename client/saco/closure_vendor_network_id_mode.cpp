#include "../raknet/NetworkTypes.h"
// Actual vendor getter. The existing comparison unit owns the static flag.
bool NetworkID::IsPeerToPeerMode(void)
{
    return peerToPeerMode;
}
