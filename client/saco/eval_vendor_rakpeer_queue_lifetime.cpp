// Expose original protected nested type names without changing any class definition or allocating views.
#include "../raknet/RakPeer.h"
struct R5PeerQueueTypeAccess : public RakPeer {
 typedef RequestedConnectionStruct Requested;
 typedef BufferedCommandStruct Buffered;
};
template DataStructures::SingleProducerConsumer<R5PeerQueueTypeAccess::Requested>::SingleProducerConsumer();
template DataStructures::SingleProducerConsumer<R5PeerQueueTypeAccess::Requested>::~SingleProducerConsumer();
template DataStructures::SingleProducerConsumer<R5PeerQueueTypeAccess::Buffered>::SingleProducerConsumer();
template DataStructures::SingleProducerConsumer<R5PeerQueueTypeAccess::Buffered>::~SingleProducerConsumer();
