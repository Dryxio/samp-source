// Explicit instantiation of the genuine vendor PlayerID queue constructor.
#include "../raknet/NetworkTypes.h"
#include "../raknet/DS_Queue.h"
template DataStructures::Queue<PlayerID>::Queue(void);
typedef char R5PlayerIDSize[(sizeof(PlayerID)==6)?1:-1];
typedef char R5PlayerIDQueueSize[(sizeof(DataStructures::Queue<PlayerID>)==16)?1:-1];
