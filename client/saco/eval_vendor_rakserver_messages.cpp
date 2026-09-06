// Complete original RakServer class; message/API methods, with R5 reduced Receive behavior verified from original.
#include <string.h>
#include <assert.h>
#include "../raknet/RakServer.h"
#include "../raknet/PacketEnumerations.h"
#include "../raknet/GetTime.h"
#include "../raknet/Rand.h"

void RakServer::InitializeSecurity( const char *pubKeyE, const char *pubKeyN )
{
	RakPeer::InitializeSecurity( 0, 0, pubKeyE, pubKeyN );
}

bool RakServer::HasPassword( void )
{
	int passwordLength;
	GetIncomingPassword(0, &passwordLength);
	return passwordLength > 0;
}

bool RakServer::Send( const char *data, const int length, PacketPriority priority, PacketReliability reliability, char orderingChannel, PlayerID playerId, bool broadcast )
{
	return RakPeer::Send( data, length, priority, reliability, orderingChannel, playerId, broadcast );
}

bool RakServer::Send( RakNet::BitStream *bitStream, PacketPriority priority, PacketReliability reliability, char orderingChannel, PlayerID playerId, bool broadcast )
{
	return RakPeer::Send( bitStream, priority, reliability, orderingChannel, playerId, broadcast );
}

Packet* RakServer::Receive( void )
{
	Packet * packet = RakPeer::Receive();
	if ( RakPeer::IsActive() && synchronizedRandomInteger )
	{
		RakNetTime time = RakNet::GetTime();

		if ( time > nextSeedUpdate || ( packet && packet->data[ 0 ] == ID_NEW_INCOMING_CONNECTION ) )
		{
			if ( time > nextSeedUpdate )
				nextSeedUpdate = time + 9000; // Seeds are updated every 9 seconds

			seed = nextSeed;

			nextSeed = randomMT();

			if ( nextSeed % 2 == 0 )   // Even
				nextSeed--; // make odd

			/*		SetRandomNumberSeedStruct s;

			s.ts = ID_TIMESTAMP;
			s.timeStamp = RakNet::GetTime();
			s.typeId = ID_SET_RANDOM_NUMBER_SEED;
			s.seed = seed;
			s.nextSeed = nextSeed;
			RakNet::BitStream s_BitS( SetRandomNumberSeedStruct_Size );
			s.Serialize( s_BitS );
			*/

			RakNet::BitStream outBitStream(sizeof(unsigned char)+sizeof(unsigned int)+sizeof(unsigned char)+sizeof(unsigned int)+sizeof(unsigned int));
			outBitStream.Write((unsigned char) ID_TIMESTAMP);
			outBitStream.Write((unsigned int) RakNet::GetTime());
			outBitStream.Write((unsigned char) ID_SET_RANDOM_NUMBER_SEED);
			outBitStream.Write(seed);
			outBitStream.Write(nextSeed);

			if ( packet && packet->data[ 0 ] == ID_NEW_INCOMING_CONNECTION )
				Send( &outBitStream, SYSTEM_PRIORITY, RELIABLE, 0, packet->playerId, false );
			else
				Send( &outBitStream, SYSTEM_PRIORITY, RELIABLE, 0, UNASSIGNED_PLAYER_ID, true );
		}
	}
	return packet;
}

void RakServer::Kick( const PlayerID playerId )
{
	RakPeer::NotifyAndFlagForDisconnect(playerId, true, 0);
}

unsigned short RakServer::GetConnectedPlayers( void )
{
	unsigned short numberOfSystems;

	RakPeer::GetConnectionList( 0, &numberOfSystems );
	return numberOfSystems;
}

bool RakServer::RPC( char* uniqueID, const char *data, unsigned int bitLength, PacketPriority priority, PacketReliability reliability, char orderingChannel, PlayerID playerId, bool broadcast, bool shiftTimestamp, NetworkID networkID, RakNet::BitStream *replyFromTarget )
{
	return RakPeer::RPC( uniqueID, data, bitLength, priority, reliability, orderingChannel, playerId, broadcast, shiftTimestamp, networkID, replyFromTarget );
}

bool RakServer::RPC( char* uniqueID, RakNet::BitStream *parameters, PacketPriority priority, PacketReliability reliability, char orderingChannel, PlayerID playerId, bool broadcast, bool shiftTimestamp, NetworkID networkID, RakNet::BitStream *replyFromTarget )
{
	return RakPeer::RPC( uniqueID, parameters, priority, reliability, orderingChannel, playerId, broadcast, shiftTimestamp, networkID, replyFromTarget );
}

bool RakServer::RPC( char* uniqueID, RakNet::BitStream *parameters, PacketPriority priority, PacketReliability reliability, char orderingChannel, PlayerID playerId, bool broadcast, bool shiftTimestamp)
{
	return RakPeer::RPC( uniqueID, parameters, priority, reliability, orderingChannel, playerId, broadcast, shiftTimestamp, UNASSIGNED_NETWORK_ID, 0 );
}

RakNet::BitStream * RakServer::GetStaticServerData( void )
{
	return RakPeer::GetRemoteStaticData( myPlayerId );
}

void RakServer::SetStaticServerData( const char *data, const int length )
{
	RakPeer::SetRemoteStaticData( myPlayerId, data, length );
}

void RakServer::SetStaticClientData( const PlayerID playerId, const char *data, const int length )
{
	RakPeer::SetRemoteStaticData( playerId, data, length );
}

bool RakServer::IsActivePlayerID( const PlayerID playerId )
{
	return RakPeer::GetRemoteSystemFromPlayerID( playerId, false, true ) != 0;
}

void RakServer::SetTimeoutTime( RakNetTime timeMS, const PlayerID target )
{
	RakPeer::SetTimeoutTime(timeMS, target);
}
