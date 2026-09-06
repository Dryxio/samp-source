// Original vendor API wrappers, complete class and original multiple-inheritance ABI.
#include <string.h>
#include "../raknet/RakServer.h"
void RakServer::Disconnect( unsigned int blockDuration, unsigned char orderingChannel )
{
	RakPeer::Disconnect( blockDuration, orderingChannel );
}

void RakServer::DeallocatePacket( Packet *packet )
{
	RakPeer::DeallocatePacket( packet );
}

void RakServer::GetPlayerIPFromID( const PlayerID playerId, char returnValue[ 22 ], unsigned short *port )
{
	*port = playerId.port;
	strcpy( returnValue, RakPeer::PlayerIDToDottedIP( playerId ) );
}

void RakServer::PingPlayer( const PlayerID playerId )
{
	RakPeer::Ping( playerId );
}

int RakServer::GetAveragePing( const PlayerID playerId )
{
	return RakPeer::GetAveragePing( playerId );
}

int RakServer::GetLastPing( const PlayerID playerId )
{
	return RakPeer::GetLastPing( playerId );
}

int RakServer::GetLowestPing( const PlayerID playerId )
{
	return RakPeer::GetLowestPing( playerId );
}

bool RakServer::IsActive( void ) const
{
	return RakPeer::IsActive();
}

bool RakServer::GenerateCompressionLayer( unsigned int inputFrequencyTable[ 256 ], bool inputLayer )
{
	return RakPeer::GenerateCompressionLayer( inputFrequencyTable, inputLayer );
}

bool RakServer::DeleteCompressionLayer( bool inputLayer )
{
	return RakPeer::DeleteCompressionLayer( inputLayer );
}

void RakServer::RegisterAsRemoteProcedureCall( char* uniqueID, void ( *functionPointer ) ( RPCParameters *rpcParms ) )
{
	RakPeer::RegisterAsRemoteProcedureCall( uniqueID, functionPointer );
}

void RakServer::RegisterClassMemberRPC( char* uniqueID, void *functionPointer )
{
	RakPeer::RegisterClassMemberRPC( uniqueID, functionPointer );
}

void RakServer::SetTrackFrequencyTable( bool b )
{
	RakPeer::SetCompileFrequencyTable( b );
}

bool RakServer::GetSendFrequencyTable( unsigned int outputFrequencyTable[ 256 ] )
{
	return RakPeer::GetOutgoingFrequencyTable( outputFrequencyTable );
}

float RakServer::GetCompressionRatio( void ) const
{
	return RakPeer::GetCompressionRatio();
}

float RakServer::GetDecompressionRatio( void ) const
{
	return RakPeer::GetDecompressionRatio();
}

void RakServer::AttachPlugin( PluginInterface *messageHandler )
{
	RakPeer::AttachPlugin(messageHandler);
}

void RakServer::DetachPlugin( PluginInterface *messageHandler )
{
	RakPeer::DetachPlugin(messageHandler);
}

void RakServer::SendStaticServerDataToClient( const PlayerID playerId )
{
	RakPeer::SendStaticData( playerId );
}

void RakServer::SetOfflinePingResponse( const char *data, const unsigned int length )
{
	RakPeer::SetOfflinePingResponse( data, length );
}

RakNet::BitStream * RakServer::GetStaticClientData( const PlayerID playerId )
{
	return RakPeer::GetRemoteStaticData( playerId );
}

unsigned int RakServer::GetNumberOfAddresses( void )
{
	return RakPeer::GetNumberOfAddresses();
}

void RakServer::PushBackPacket( Packet *packet, bool pushAtHead )
{
	RakPeer::PushBackPacket(packet, pushAtHead);
}

const char* RakServer::GetLocalIP( unsigned int index )
{
	return RakPeer::GetLocalIP( index );
}

int RakServer::GetIndexFromPlayerID( const PlayerID playerId )
{
	return RakPeer::GetIndexFromPlayerID( playerId );
}

PlayerID RakServer::GetPlayerIDFromIndex( int index )
{
	return RakPeer::GetPlayerIDFromIndex( index );
}

void RakServer::RemoveFromBanList( const char *IP )
{
	RakPeer::RemoveFromBanList( IP );
}

void RakServer::ClearBanList( void )
{
	RakPeer::ClearBanList();
}

bool RakServer::IsBanned( const char *IP )
{
	return RakPeer::IsBanned( IP );
}

RakNetStatisticsStruct * const RakServer::GetStatistics( const PlayerID playerId )
{
	return RakPeer::GetStatistics( playerId );
}

bool RakServer::IsNetworkSimulatorActive( void )
{
	return RakPeer::IsNetworkSimulatorActive();
}

