// Actual client packet/RPC/static-data methods; complete real class header, original source order.
#include <string.h>
#include <assert.h>
#include "../raknet/RakClient.h"
#include "../raknet/PacketEnumerations.h"
#include "../raknet/GetTime.h"

bool RakClient::Send( const char *data, const int length, PacketPriority priority, PacketReliability reliability, char orderingChannel )
{
	if ( remoteSystemList == 0 )
		return false;
		
	return RakPeer::Send( data, length, priority, reliability, orderingChannel, remoteSystemList[ 0 ].playerId, false );
}

bool RakClient::Send( RakNet::BitStream * bitStream, PacketPriority priority, PacketReliability reliability, char orderingChannel )
{
	if ( remoteSystemList == 0 )
		return false;
		
	return RakPeer::Send( bitStream, priority, reliability, orderingChannel, remoteSystemList[ 0 ].playerId, false );
}

Packet* RakClient::Receive( void )
{
	Packet * packet = RakPeer::Receive();
	
	// Intercept specific client / server feature packets
	
	if ( packet )
	{
		RakNet::BitStream bitStream( packet->data, packet->length, false );
		int i;
		
		if ( packet->data[ 0 ] == ID_CONNECTION_REQUEST_ACCEPTED )
		{
//			ConnectionAcceptStruct cas;
//			cas.Deserialize(bitStream);
		//	unsigned short remotePort;
		//	PlayerID externalID;
			PlayerIndex playerIndex;

			RakNet::BitStream inBitStream(packet->data, packet->length, false);
			inBitStream.IgnoreBits(8); // ID_CONNECTION_REQUEST_ACCEPTED
			inBitStream.IgnoreBits(8 * sizeof(unsigned short)); //inBitStream.Read(remotePort);
			inBitStream.IgnoreBits(8 * sizeof(unsigned int)); //inBitStream.Read(externalID.binaryAddress);
			inBitStream.IgnoreBits(8 * sizeof(unsigned short)); //inBitStream.Read(externalID.port);			
			inBitStream.Read(playerIndex);

			localPlayerIndex = playerIndex;
			packet->playerIndex = playerIndex;
		}
		else if (
				packet->data[ 0 ] == ID_REMOTE_NEW_INCOMING_CONNECTION ||
				packet->data[ 0 ] == ID_REMOTE_EXISTING_CONNECTION ||
				packet->data[ 0 ] == ID_REMOTE_DISCONNECTION_NOTIFICATION ||
				packet->data[ 0 ] == ID_REMOTE_CONNECTION_LOST )
		{
			bitStream.IgnoreBits( 8 ); // Ignore identifier
			bitStream.Read( packet->playerId.binaryAddress );
			bitStream.Read( packet->playerId.port );
			
			if ( bitStream.Read( ( unsigned short& ) packet->playerIndex ) == false )
			{
				DeallocatePacket( packet );
				return 0;
			}
			
			
			if ( packet->data[ 0 ] == ID_REMOTE_DISCONNECTION_NOTIFICATION ||
				packet->data[ 0 ] == ID_REMOTE_CONNECTION_LOST )
			{
				i = GetOtherClientIndexByPlayerID( packet->playerId );
				
				if ( i >= 0 )
					otherClients[ i ].isActive = false;
			}
		}			
		else if ( packet->data[ 0 ] == ID_REMOTE_STATIC_DATA )
		{
			bitStream.IgnoreBits( 8 ); // Ignore identifier
			bitStream.Read( packet->playerId.binaryAddress );
			bitStream.Read( packet->playerId.port );
			bitStream.Read( packet->playerIndex ); // ADDED BY KURI 
			
			i = GetOtherClientIndexByPlayerID( packet->playerId );
			
			if ( i < 0 )
				i = GetFreeOtherClientIndex();
				
			if ( i >= 0 )
			{
				otherClients[ i ].playerId = packet->playerId;
				otherClients[ i ].isActive = true;
				otherClients[ i ].staticData.Reset();
				// The static data is what is left over in the stream
				otherClients[ i ].staticData.Write( ( char* ) bitStream.GetData() + BITS_TO_BYTES( bitStream.GetReadOffset() ), bitStream.GetNumberOfBytesUsed() - BITS_TO_BYTES( bitStream.GetReadOffset() ) );
			}
		}
		else if ( packet->data[ 0 ] == ID_BROADCAST_PINGS )
		{
			PlayerID playerId;
			int index;

			bitStream.IgnoreBits( 8 ); // Ignore identifier
			
			for ( i = 0; i < 32; i++ )
			{
				if ( bitStream.Read( playerId.binaryAddress ) == false )
					break; // No remaining data!
					
				bitStream.Read( playerId.port );
				
				index = GetOtherClientIndexByPlayerID( playerId );
				
				if ( index >= 0 )
					bitStream.Read( otherClients[ index ].ping );
				else
				{
					index = GetFreeOtherClientIndex();
					
					if ( index >= 0 )
					{
						otherClients[ index ].isActive = true;
						bitStream.Read( otherClients[ index ].ping );
						otherClients[ index ].playerId = playerId;
						otherClients[ index ].staticData.Reset();
					}
					
					else
						bitStream.IgnoreBits( sizeof( short ) * 8 );
				}
			}
			
			DeallocatePacket( packet );
			return 0;
		}
        else
		if ( packet->data[ 0 ] == ID_TIMESTAMP &&
			packet->length == sizeof(unsigned char)+sizeof(unsigned int)+sizeof(unsigned char)+sizeof(unsigned int)+sizeof(unsigned int) )
		{
			

			RakNet::BitStream inBitStream(packet->data, packet->length, false);
		
			RakNetTime timeStamp;
			unsigned char typeId;
			unsigned int in_seed;
			unsigned int in_nextSeed;
			inBitStream.IgnoreBits(8); // ID_TIMESTAMP
			inBitStream.Read(timeStamp);
			inBitStream.Read(typeId); // ID_SET_RANDOM_NUMBER_SEED ?

			// Check to see if this is a user TIMESTAMP message which
			// accidentally has length SetRandomNumberSeedStruct_Size
			if ( typeId != ID_SET_RANDOM_NUMBER_SEED )
				return packet;

			inBitStream.Read(in_seed);
			inBitStream.Read(in_nextSeed);
			
			seed = in_seed;
			nextSeed = in_nextSeed;
			nextSeedUpdate = timeStamp + 9000; // Seeds are updated every 9 seconds
			
			DeallocatePacket( packet );
			return 0;
		}
	}
	
	return packet;
}

bool RakClient::RPC( char* uniqueID, const char *data, unsigned int bitLength, PacketPriority priority, PacketReliability reliability, char orderingChannel, bool shiftTimestamp, NetworkID networkID, RakNet::BitStream *replyFromTarget )
{
	if ( remoteSystemList == 0 )
		return false;
		
	return RakPeer::RPC( uniqueID, data, bitLength, priority, reliability, orderingChannel, remoteSystemList[ 0 ].playerId, false, shiftTimestamp, networkID, replyFromTarget );
}

bool RakClient::RPC( char* uniqueID, RakNet::BitStream *parameters, PacketPriority priority, PacketReliability reliability, char orderingChannel, bool shiftTimestamp, NetworkID networkID, RakNet::BitStream *replyFromTarget )
{
	if ( remoteSystemList == 0 )
		return false;
		
	return RakPeer::RPC( uniqueID, parameters, priority, reliability, orderingChannel, remoteSystemList[ 0 ].playerId, false, shiftTimestamp, networkID, replyFromTarget );
}

bool RakClient::RPC( char* uniqueID, RakNet::BitStream *parameters, PacketPriority priority, PacketReliability reliability, char orderingChannel, bool shiftTimestamp )
{
	if ( remoteSystemList == 0 )
		return false;
		
	return RakPeer::RPC( uniqueID, parameters, priority, reliability, orderingChannel, remoteSystemList[ 0 ].playerId, false, shiftTimestamp, UNASSIGNED_NETWORK_ID, 0 );
}

RakNet::BitStream * RakClient::GetStaticServerData( void )
{
	if ( remoteSystemList == 0 )
		return 0;
		
	return RakPeer::GetRemoteStaticData( remoteSystemList[ 0 ].playerId );
}

void RakClient::SetStaticServerData( const char *data, const int length )
{
	if ( remoteSystemList == 0 )
		return ;
		
	RakPeer::SetRemoteStaticData( remoteSystemList[ 0 ].playerId, data, length );
}

RakNet::BitStream * RakClient::GetStaticClientData( const PlayerID playerId )
{
	int i;
	
	if ( playerId == UNASSIGNED_PLAYER_ID )
	{
		return & localStaticData;
	}
	
	else
	{
		i = GetOtherClientIndexByPlayerID( playerId );
		
		if ( i >= 0 )
		{
			return & ( otherClients[ i ].staticData );
		}

	}
	
	return 0;
}

void RakClient::SetStaticClientData( const PlayerID playerId, const char *data, const int length )
{
	int i;
	
	if ( playerId == UNASSIGNED_PLAYER_ID )
	{
		localStaticData.Reset();
		localStaticData.Write( data, length );
	}
	
	else
	{
		i = GetOtherClientIndexByPlayerID( playerId );
		
		if ( i >= 0 )
		{
			otherClients[ i ].staticData.Reset();
			otherClients[ i ].staticData.Write( data, length );
		}
		
		else
			RakPeer::SetRemoteStaticData( playerId, data, length );
	}
	
}

void RakClient::SendStaticClientDataToServer( void )
{
	if ( remoteSystemList == 0 )
		return ;
		
	RakPeer::SendStaticData( remoteSystemList[ 0 ].playerId );
}

int RakClient::GetOtherClientIndexByPlayerID( const PlayerID playerId )
{
	unsigned i;
	
	for ( i = 0; i < 32; i++ )
	{
		if ( otherClients[ i ].playerId == playerId )
			return i;
	}
	
	return -1;
}

int RakClient::GetFreeOtherClientIndex( void )
{
	unsigned i;
	
	for ( i = 0; i < 32; i++ )
	{
		if ( otherClients[ i ].isActive == false )
			return i;
	}
	
	return -1;
}
