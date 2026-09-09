// Direct vendor factory virtual destruction, statically typed as the genuine RakPeer.
#include "../raknet/RakNetworkFactory.h"
#include "../raknet/RakPeer.h"
void RakNetworkFactory::DestroyRakPeerInterface(RakPeerInterface* i)
{
    delete (RakPeer*)i;
}
