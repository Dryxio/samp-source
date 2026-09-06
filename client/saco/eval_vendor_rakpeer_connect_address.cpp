// Actual RakPeer direct connect and address access; full source class.
#include <string.h>
#include <assert.h>
#include "../raknet/RakPeer.h"

bool RakPeer::Connect( const char* host, unsigned short remotePort, char* passwordData, int passwordDataLength )
{
	// If endThreads is true here you didn't call Initialize() first.
	if ( host == 0 || endThreads || connectionSocket == INVALID_SOCKET )
		return false;

	unsigned numberOfFreeSlots;

	numberOfFreeSlots = 0;

	//if (passwordDataLength>MAX_OFFLINE_DATA_LENGTH)
	//	passwordDataLength=MAX_OFFLINE_DATA_LENGTH;
	if (passwordDataLength>255)
		passwordDataLength=255;

	if (passwordData==0)
		passwordDataLength=0;

	// Not threadsafe but it's not important enough to lock.  Who is going to change the password a lot during runtime?
	// It won't overflow at least because outgoingPasswordLength is an unsigned char
//	if (passwordDataLength>0)
//		memcpy(outgoingPassword, passwordData, passwordDataLength);
//	outgoingPasswordLength=(unsigned char) passwordDataLength;

	// If the host starts with something other than 0, 1, or 2 it's (probably) a domain name.
	if ( host[ 0 ] < '0' || host[ 0 ] > '2' )
	{
#if !defined(_COMPATIBILITY_1)
		host = ( char* ) SocketLayer::Instance()->DomainNameToIP( host );
#else
		return false;
#endif
		if (host==0)
			return false;
	}

	// Connecting to ourselves in the same instance of the program?
	if ( ( strcmp( host, "127.0.0.1" ) == 0 || strcmp( host, "0.0.0.0" ) == 0 ) && remotePort == myPlayerId.port )
		return false;

	return SendConnectionRequest( host, remotePort, passwordData, passwordDataLength );
}

unsigned int RakPeer::GetNumberOfAddresses( void )
{
#if !defined(_COMPATIBILITY_1)
	char ipList[ 10 ][ 16 ];
	memset( ipList, 0, sizeof( char ) * 16 * 10 );
	SocketLayer::Instance()->GetMyIP( ipList );

	int i = 0;

	while ( ipList[ i ][ 0 ] )
		i++;

	return i;
#else
	assert(0);
	return 0;
#endif
}

const char* RakPeer::PlayerIDToDottedIP( const PlayerID playerId ) const
{
#if !defined(_COMPATIBILITY_1)
	in_addr in;
	in.s_addr = playerId.binaryAddress;
	return inet_ntoa( in );
#else
	assert(0); // Not supported
	return 0;
#endif
}

const char* RakPeer::GetLocalIP( unsigned int index )
{
#if !defined(_COMPATIBILITY_1)
	static char ipList[ 10 ][ 16 ];

	if ( index >= 10 )
		index = 9;

	memset( ipList, 0, sizeof( char ) * 16 * 10 );

	SocketLayer::Instance()->GetMyIP( ipList );

	return ipList[ index ];
#else
	assert(0);
	return 0;
#endif
}
