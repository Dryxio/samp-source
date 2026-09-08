// Direct vendor template members used by RakPeer::packetSingleProducerConsumer.
#include "../raknet/NetworkTypes.h"
#include "../raknet/SingleProducerConsumer.h"
template DataStructures::SingleProducerConsumer<Packet*>::SingleProducerConsumer();
template DataStructures::SingleProducerConsumer<Packet*>::~SingleProducerConsumer();
typedef char R5PacketSPCSize[(sizeof(DataStructures::SingleProducerConsumer<Packet*>)==24)?1:-1];
