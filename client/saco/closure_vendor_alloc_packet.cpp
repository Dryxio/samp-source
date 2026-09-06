#include "../raknet/NetworkTypes.h"
#include <stdlib.h>

// Original complete contiguous Packet allocation from RakPeer.cpp.
Packet *AllocPacket(unsigned dataSize)
{
	Packet *p = (Packet *)malloc(sizeof(Packet)+dataSize);
	p->data=(unsigned char*)p+sizeof(Packet);
	p->length=dataSize;
	p->deleteData=false;
	return p;
}

typedef char R5PacketSize[(sizeof(Packet)==21)?1:-1];
