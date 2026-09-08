// Actual vendor queue/list members; original RakPeer constructor and ReliabilityLayer::Update callers identify these types.
#include "../raknet/NetworkTypes.h"
#include "../raknet/DS_Queue.h"
// Declaration access only: expose the actual nested vendor type for VC8 explicit instantiation.
#define private public
#include "../raknet/ReliabilityLayer.h"
#undef private
template DataStructures::Queue<Packet*>::Queue();
template void DataStructures::List<ReliabilityLayer::DataAndTime*>::Insert(ReliabilityLayer::DataAndTime* const);
