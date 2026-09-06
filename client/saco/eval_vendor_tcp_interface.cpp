// Full original vendor TCP source; static matching only, no socket or thread execution.
#include "../raknet/TCPInterface.cpp"

typedef char R5TCPInterfaceSize[(sizeof(TCPInterface)==167)?1:-1];
typedef char R5RemoteClientSize[(sizeof(RemoteClient)==10)?1:-1];
typedef char R5TCPQueueSize[(sizeof(DataStructures::SingleProducerConsumer<Packet>)==24)?1:-1];
