// Original RakPeer predicate, emitted separately from calling RakServer context.
#include "../raknet/RakPeer.h"
bool RakPeer::IsActive(void) const
{
    return endThreads == false;
}
