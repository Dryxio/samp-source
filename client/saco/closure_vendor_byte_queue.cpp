// Direct complete vendor ByteQueue implementation; original circular-buffer semantics.
#include "../raknet/DS_ByteQueue.cpp"
typedef char R5ByteQueueComplete[(sizeof(DataStructures::ByteQueue)==16)?1:-1];
