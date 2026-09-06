#include "../raknet/InternalPacketPool.h"
#include <assert.h>

InternalPacketPool::InternalPacketPool()
{	
	// Speed things up by not reallocating at runtime when a mutex is locked.
	pool.ClearAndForceAllocation( 64 );
	unsigned i;
	for (i=0; i < 64; i++)
		pool.Push(new InternalPacket);
}

InternalPacketPool::~InternalPacketPool()
{
	ClearPool();
}

void InternalPacketPool::ClearPool( void )
{
	while ( pool.Size() )
		delete pool.Pop();
}

typedef char R5CompleteInternalPacketSize[(sizeof(InternalPacket)==55)?1:-1];
typedef char R5CompletePacketPoolSize[(sizeof(InternalPacketPool)==16)?1:-1];
