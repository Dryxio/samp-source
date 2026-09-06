#include "../raknet/InternalPacketPool.h"
#include <assert.h>

void InternalPacketPool::ReleasePointer( InternalPacket *p )
{
	if ( p == 0 )
	{
		// Releasing a null pointer?
#ifdef _DEBUG
		assert( 0 );
#endif
		return ;
	}
	
#ifdef _DEBUG
	p->data=0;
#endif
	//poolMutex.Lock();
	pool.Push( p );
	//poolMutex.Unlock();
}
