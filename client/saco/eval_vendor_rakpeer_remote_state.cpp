#include <string.h>
#include <assert.h>
#include "../raknet/RakPeer.h"
#include "../raknet/RakNetDefines.h"
#include "../raknet/GetTime.h"

int PlayerIDAndIndexComp( const PlayerID &key, const PlayerIDAndIndex &data )
{
	if (key < data.playerId)
		return -1;
	if (key==data.playerId)
		return 0;
	return 1;
}

bool RakPeer::GetConnectionList( PlayerID *remoteSystems, unsigned short *numberOfSystems ) const
{
	int count, index;
	count=0;

	if ( remoteSystemList == 0 || endThreads == true )
	{
		*numberOfSystems = 0;
		return false;
	}

	// This is called a lot so I unrolled the loop
	if ( remoteSystems )
	{
		// remoteSystemList in user thread
		//for ( count = 0, index = 0; index < remoteSystemListSize; ++index )
		for ( count = 0, index = 0; index < maximumNumberOfPeers; ++index )
			if ( remoteSystemList[ index ].isActive && remoteSystemList[ index ].connectMode==RemoteSystemStruct::CONNECTED)
			{
				if ( count < *numberOfSystems )
					remoteSystems[ count ] = remoteSystemList[ index ].playerId;

				++count;
			}
	}
	else
	{
		// remoteSystemList in user thread
		//for ( count = 0, index = 0; index < remoteSystemListSize; ++index )
		for ( count = 0, index = 0; index < maximumNumberOfPeers; ++index )
			if ( remoteSystemList[ index ].isActive && remoteSystemList[ index ].connectMode==RemoteSystemStruct::CONNECTED)
				++count;
	}

	*numberOfSystems = ( unsigned short ) count;

	return 0;
}

int RakPeer::GetIndexFromPlayerID( const PlayerID playerId )
{
	return GetIndexFromPlayerID(playerId, false);
}

PlayerID RakPeer::GetPlayerIDFromIndex( int index )
{
	// remoteSystemList in user thread
	//if ( index >= 0 && index < remoteSystemListSize )
	if ( index >= 0 && index < maximumNumberOfPeers )
		if (remoteSystemList[ index ].connectMode==RakPeer::RemoteSystemStruct::CONNECTED) // Don't give the user players that aren't fully connected, since sends will fail
			return remoteSystemList[ index ].playerId;

	return UNASSIGNED_PLAYER_ID;
}

PlayerID RakPeer::GetAnyPlayerIDFromIndex( int index )
{
	// remoteSystemList in user thread
	if ( index >= 0 && index < maximumNumberOfPeers )
		return remoteSystemList[ index ].playerId;

	return UNASSIGNED_PLAYER_ID;
}

int RakPeer::GetAveragePing( const PlayerID playerId )
{
	int sum, quantity;
	RemoteSystemStruct *remoteSystem = GetRemoteSystemFromPlayerID( playerId, false, false );

	if ( remoteSystem == 0 )
		return -1;

	for ( sum = 0, quantity = 0; quantity < PING_TIMES_ARRAY_SIZE; quantity++ )
	{
		if ( remoteSystem->pingAndClockDifferential[ quantity ].pingTime == 65535 )
			break;
		else
			sum += remoteSystem->pingAndClockDifferential[ quantity ].pingTime;
	}

	if ( quantity > 0 )
		return sum / quantity;
	else
		return -1;
}

int RakPeer::GetLastPing( const PlayerID playerId ) const
{
	RemoteSystemStruct * remoteSystem = GetRemoteSystemFromPlayerID( playerId, false, false );

	if ( remoteSystem == 0 )
		return -1;

	if ( remoteSystem->pingAndClockDifferentialWriteIndex == 0 )
		return remoteSystem->pingAndClockDifferential[ PING_TIMES_ARRAY_SIZE - 1 ].pingTime;
	else
		return remoteSystem->pingAndClockDifferential[ remoteSystem->pingAndClockDifferentialWriteIndex - 1 ].pingTime;
}

int RakPeer::GetLowestPing( const PlayerID playerId ) const
{
	RemoteSystemStruct * remoteSystem = GetRemoteSystemFromPlayerID( playerId, false, false );

	if ( remoteSystem == 0 )
		return -1;

	return remoteSystem->lowestPing;
}

PlayerID RakPeer::GetExternalID( const PlayerID target ) const
{
	unsigned i;
	PlayerID inactiveExternalId;

	inactiveExternalId=UNASSIGNED_PLAYER_ID;

	// First check for active connection with this playerId
	for ( i = 0; i < maximumNumberOfPeers; i++ )
	{
		if (remoteSystemList[ i ].playerId == target || target==UNASSIGNED_PLAYER_ID )
		{
			if ( remoteSystemList[ i ].isActive )
				return remoteSystemList[ i ].myExternalPlayerId;
			else
				inactiveExternalId=remoteSystemList[ i ].myExternalPlayerId;
		}
	}

	return inactiveExternalId;
}

void RakPeer::SetTimeoutTime( RakNetTime timeMS, const PlayerID target )
{
	RemoteSystemStruct * remoteSystem = GetRemoteSystemFromPlayerID( target, false, true );

	if ( remoteSystem != 0 )
		remoteSystem->reliabilityLayer.SetTimeoutTime(timeMS);
}

RakNetStatisticsStruct * const RakPeer::GetStatistics( const PlayerID playerId )
{
	if (playerId==UNASSIGNED_PLAYER_ID)
	{
		bool firstWrite=true;
		static RakNetStatisticsStruct sum;
		RakNetStatisticsStruct *systemStats;
		// Return a crude sum
		for ( unsigned short i = 0; i < maximumNumberOfPeers; i++ )
		{
			if (remoteSystemList[ i ].isActive)
			{
				systemStats=remoteSystemList[ i ].reliabilityLayer.GetStatistics( true );
				
				if (firstWrite==true)
				{
					memcpy(&sum, systemStats, sizeof(RakNetStatisticsStruct));
					firstWrite=false;
				}
				else
					sum+=*systemStats;
			}
		}
		return &sum;
	}
	else
	{
		RemoteSystemStruct * rss;
	rss = GetRemoteSystemFromPlayerID( playerId, false, false );
		if ( rss && endThreads==false )
			return rss->reliabilityLayer.GetStatistics( true );
	}	

	return 0;
}

int RakPeer::GetIndexFromPlayerID( const PlayerID playerId, bool calledFromNetworkThread )
{
	unsigned i;

	if ( playerId == UNASSIGNED_PLAYER_ID )
		return -1;

	if (calledFromNetworkThread)
	{
		bool objectExists;
		unsigned index;
		index = remoteSystemLookup.GetIndexFromKey(playerId, &objectExists);
		if (objectExists)
		{
			assert(remoteSystemList[remoteSystemLookup[index].index].playerId==playerId);
			return remoteSystemLookup[index].index;
		}
		else
			return -1;
	}
	else
	{
		// remoteSystemList in user and network thread
		for ( i = 0; i < maximumNumberOfPeers; i++ )
			if ( remoteSystemList[ i ].isActive && remoteSystemList[ i ].playerId == playerId )
				return i;
	}

	return -1;
}

RakPeer::RemoteSystemStruct *RakPeer::GetRemoteSystemFromPlayerID( const PlayerID playerID, bool calledFromNetworkThread, bool onlyActive ) const
{
	unsigned i;

	if ( playerID == UNASSIGNED_PLAYER_ID )
		return 0;

	if (calledFromNetworkThread)
	{
		bool objectExists;
		unsigned index;
		index = remoteSystemLookup.GetIndexFromKey(playerID, &objectExists);
		if (objectExists)
		{
#ifdef _DEBUG
			assert(remoteSystemList[ remoteSystemLookup[index].index ].playerId==playerID);
#endif
			return remoteSystemList + remoteSystemLookup[index].index;
		}
	}
	else
	{
		int deadConnectionIndex=-1;

		// Active connections take priority.  But if there are no active connections, return the first systemAddress match found
		for ( i = 0; i < maximumNumberOfPeers; i++ )
		{
			if (remoteSystemList[ i ].playerId == playerID )
			{
				if ( remoteSystemList[ i ].isActive )
					return remoteSystemList + i;
				else if (deadConnectionIndex==-1)
					deadConnectionIndex=i;
			}
		}

		if (deadConnectionIndex!=-1 && onlyActive==false)
			return remoteSystemList + deadConnectionIndex;
	}

	return 0;
}

unsigned short RakPeer::GetNumberOfRemoteInitiatedConnections( void ) const
{
	unsigned short i, numberOfIncomingConnections;

	if ( remoteSystemList == 0 || endThreads == true )
		return 0;

	numberOfIncomingConnections = 0;

	// remoteSystemList in network thread
	for ( i = 0; i < maximumNumberOfPeers; i++ )
	//for ( i = 0; i < remoteSystemListSize; i++ )
	{
		if ( remoteSystemList[ i ].isActive && remoteSystemList[ i ].weInitiatedTheConnection == false && remoteSystemList[i].connectMode==RemoteSystemStruct::CONNECTED)
			numberOfIncomingConnections++;
	}

	return numberOfIncomingConnections;
}

RakPeer::RemoteSystemStruct * RakPeer::AssignPlayerIDToRemoteSystemList( const PlayerID playerId, RemoteSystemStruct::ConnectMode connectionMode )
{
	RemoteSystemStruct * remoteSystem;
	unsigned i,j;
	RakNetTime time = RakNet::GetTime();
#ifdef _DEBUG
	assert(playerId!=UNASSIGNED_PLAYER_ID);
#endif

	// remoteSystemList in user thread
	for ( i = 0; i < maximumNumberOfPeers; i++ )
	//for ( i = 0; i < remoteSystemListSize; i++ )
	{
		if ( remoteSystemList[ i ].isActive==false )
		{
			remoteSystem=remoteSystemList+i;
			remoteSystem->rpcMap.Clear();
			remoteSystem->playerId = playerId;
			remoteSystem->isActive=true; // This one line causes future incoming packets to go through the reliability layer
			remoteSystem->reliabilityLayer.SetSplitMessageProgressInterval(splitMessageProgressInterval);
			remoteSystem->reliabilityLayer.SetUnreliableTimeout(unreliableTimeout);
			remoteSystem->reliabilityLayer.SetEncryptionKey( 0 );

			for ( j = 0; j < (unsigned) PING_TIMES_ARRAY_SIZE; j++ )
			{
				remoteSystem->pingAndClockDifferential[ j ].pingTime = 65535;
				remoteSystem->pingAndClockDifferential[ j ].clockDifferential = 0;
			}

			remoteSystem->connectMode=connectionMode;
			remoteSystem->pingAndClockDifferentialWriteIndex = 0;
			remoteSystem->lowestPing = 65535;
			remoteSystem->nextPingTime = 0; // Ping immediately
			remoteSystem->weInitiatedTheConnection = false;
			remoteSystem->staticData.Reset();
			remoteSystem->connectionTime = time;
			remoteSystem->myExternalPlayerId = UNASSIGNED_PLAYER_ID;
			remoteSystem->setAESKey=false;
			remoteSystem->lastReliableSend=time;

			remoteSystem->field_C66 = 0;
			remoteSystem->field_C67 = 0;
			remoteSystem->field_C68 = 0;

			// Reserve this reliability layer for ourselves.
			remoteSystem->reliabilityLayer.Reset(true);

			/// Add this player to the lookup tree
			PlayerIDAndIndex playerIDAndIndex;
			playerIDAndIndex.playerId=playerId;
			playerIDAndIndex.index=i;
			remoteSystemLookup.Insert(playerId,playerIDAndIndex);

			return remoteSystem;
		}
	}

	return 0;
}

void RakPeer::ShiftIncomingTimestamp( unsigned char *data, PlayerID playerId ) const
{
#ifdef _DEBUG
	assert( IsActive() );
	assert( data );
#endif

	RakNet::BitStream timeBS( data, sizeof(RakNetTime), false);
	RakNetTime encodedTimestamp;
	timeBS.Read(encodedTimestamp);

	encodedTimestamp = encodedTimestamp - GetBestClockDifferential( playerId );
	timeBS.SetWriteOffset(0);
	timeBS.Write(encodedTimestamp);
}

RakNetTime RakPeer::GetBestClockDifferential( const PlayerID playerId ) const
{
	int counter, lowestPingSoFar;
	RakNetTime clockDifferential;
	RemoteSystemStruct *remoteSystem = GetRemoteSystemFromPlayerID( playerId, true, true );

	if ( remoteSystem == 0 )
		return 0;

	lowestPingSoFar = 65535;

	clockDifferential = 0;

	for ( counter = 0; counter < PING_TIMES_ARRAY_SIZE; counter++ )
	{
		if ( remoteSystem->pingAndClockDifferential[ counter ].pingTime == 65535 )
			break;

		if ( remoteSystem->pingAndClockDifferential[ counter ].pingTime < lowestPingSoFar )
		{
			clockDifferential = remoteSystem->pingAndClockDifferential[ counter ].clockDifferential;
			lowestPingSoFar = remoteSystem->pingAndClockDifferential[ counter ].pingTime;
		}
	}

	return clockDifferential;
}

bool RakPeer::AllowIncomingConnections(void) const
{
	return GetNumberOfRemoteInitiatedConnections() < GetMaximumIncomingConnections();
}

bool RakPeer::ValidSendTarget(PlayerID playerId, bool broadcast)
{
	unsigned remoteSystemIndex;

	// remoteSystemList in user thread.  This is slow so only do it in debug
	for ( remoteSystemIndex = 0; remoteSystemIndex < maximumNumberOfPeers; remoteSystemIndex++ )
	//for ( remoteSystemIndex = 0; remoteSystemIndex < remoteSystemListSize; remoteSystemIndex++ )
	{
		if ( remoteSystemList[ remoteSystemIndex ].isActive &&
			remoteSystemList[ remoteSystemIndex ].connectMode==RakPeer::RemoteSystemStruct::CONNECTED && // Not fully connected players are not valid user-send targets because the reliability layer wasn't reset yet
			( ( broadcast == false && remoteSystemList[ remoteSystemIndex ].playerId == playerId ) ||
			( broadcast == true && remoteSystemList[ remoteSystemIndex ].playerId != playerId ) )
			)
			return true;
	}

	return false;
}
