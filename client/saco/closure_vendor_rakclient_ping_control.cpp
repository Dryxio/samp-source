#include <string.h>
#include "../raknet/RakClient.h"

// Direct vendor method bodies; separate provider unit preserves call boundaries.
void RakClient::StartOccasionalPing( void )
{
	RakPeer::SetOccasionalPing( true );
}

void RakClient::StopOccasionalPing( void )
{
	RakPeer::SetOccasionalPing( false );
}
