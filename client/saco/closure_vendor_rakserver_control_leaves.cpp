#include <string.h>
#include "../raknet/RakServer.h"

// Direct vendor method bodies; separate provider unit preserves call boundaries.
void RakServer::SetPassword( const char *_password )
{
	if ( _password )
	{
		RakPeer::SetIncomingPassword( _password, ( int ) strlen( _password ) + 1 );
	}

	else
	{
		RakPeer::SetIncomingPassword( 0, 0 );
	}
}

void RakServer::StartOccasionalPing( void )
{
	RakPeer::SetOccasionalPing( true );
}

void RakServer::StopOccasionalPing( void )
{
	RakPeer::SetOccasionalPing( false );
}

unsigned int RakServer::GetSynchronizedRandomInteger( void ) const
{
	return seed;
}

void RakServer::StartSynchronizedRandomInteger( void )
{
	synchronizedRandomInteger = true;
}

void RakServer::StopSynchronizedRandomInteger( void )
{
	synchronizedRandomInteger = false;
}

void RakServer::SetRelayStaticClientData( bool b )
{
	relayStaticClientData = b;
}
