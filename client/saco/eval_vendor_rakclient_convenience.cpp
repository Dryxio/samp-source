// Unchanged vendor methods, complete actual RakClient class and multiple-inheritance ABI.
#include <string.h>
#include "../raknet/RakClient.h"
#include "../raknet/GetTime.h"

void RakClient::InitializeSecurity( const char *privKeyP, const char *privKeyQ )
{
	RakPeer::InitializeSecurity( privKeyP, privKeyQ, 0, 0 );
}

void RakClient::SetPassword( const char *_password )
{
	if ( _password == 0 || _password[ 0 ] == 0 )
		password.Reset();
	else
	{
		password.Reset();
		password.Write( _password, ( int ) strlen( _password ) + 1 );
	}
}

bool RakClient::HasPassword( void ) const
{
	return password.GetNumberOfBytesUsed() > 0;
}

int RakClient::GetPlayerPing( const PlayerID playerId )
{
	int i;
	
	for ( i = 0; i < 32; i++ )
		if ( otherClients[ i ].playerId == playerId )
			return otherClients[ i ].ping;
			
	return -1;
}

bool RakClient::IsConnected( void ) const
{
	unsigned short numberOfSystems;
	
	RakPeer::GetConnectionList( 0, &numberOfSystems );
	return numberOfSystems == 1;
}

unsigned int RakClient::GetSynchronizedRandomInteger( void ) const
{
	if ( RakNet::GetTime() > nextSeedUpdate )
		return nextSeed;
	else
		return seed;
}

PlayerID RakClient::GetServerID( void ) const
{
	if ( remoteSystemList == 0 )
		return UNASSIGNED_PLAYER_ID;
		
	return remoteSystemList[ 0 ].playerId;
}

PlayerID RakClient::GetPlayerID( void ) const
{
	if ( remoteSystemList == 0 )
		return UNASSIGNED_PLAYER_ID;
		
	// GetExternalID is more accurate because it reflects our external IP and port to the server.
	// GetInternalID only matches the parameters we passed
	PlayerID myID = RakPeer::GetExternalID( remoteSystemList[ 0 ].playerId );
	
	if ( myID == UNASSIGNED_PLAYER_ID )
		return RakPeer::GetInternalID();
	else
		return myID;
}

void RakClient::SetTimeoutTime( RakNetTime timeMS )
{
	RakPeer::SetTimeoutTime( timeMS, GetServerID() );
}

RakNetStatisticsStruct* const RakClient::GetStatistics( void )
{
	return RakPeer::GetStatistics( remoteSystemList[ 0 ].playerId );
}

PlayerIndex RakClient::GetPlayerIndex( void )
{
	return localPlayerIndex;
}
