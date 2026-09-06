// Original vendor API wrappers, complete class and original multiple-inheritance ABI.
#include <string.h>
#include "../raknet/RakClient.h"
void RakClient::Disconnect( unsigned int blockDuration, unsigned char orderingChannel )
{
	RakPeer::Disconnect( blockDuration, orderingChannel );
}

void RakClient::DeallocatePacket( Packet *packet )
{
	RakPeer::DeallocatePacket( packet );
}

void RakClient::PingServer( void )
{
	if ( remoteSystemList == 0 )
		return ;
		
	RakPeer::Ping( remoteSystemList[ 0 ].playerId );
}

int RakClient::GetAveragePing( void )
{
	if ( remoteSystemList == 0 )
		return -1;
		
	return RakPeer::GetAveragePing( remoteSystemList[ 0 ].playerId );
}

int RakClient::GetLastPing( void ) const
{
	if ( remoteSystemList == 0 )
		return -1;
		
	return RakPeer::GetLastPing( remoteSystemList[ 0 ].playerId );
}

int RakClient::GetLowestPing( void ) const
{
	if ( remoteSystemList == 0 )
		return -1;
		
	return RakPeer::GetLowestPing( remoteSystemList[ 0 ].playerId );
}

bool RakClient::GenerateCompressionLayer( unsigned int inputFrequencyTable[ 256 ], bool inputLayer )
{
	return RakPeer::GenerateCompressionLayer( inputFrequencyTable, inputLayer );
}

bool RakClient::DeleteCompressionLayer( bool inputLayer )
{
	return RakPeer::DeleteCompressionLayer( inputLayer );
}

void RakClient::RegisterAsRemoteProcedureCall( char* uniqueID, void ( *functionPointer ) ( RPCParameters *rpcParms ) )
{
	RakPeer::RegisterAsRemoteProcedureCall( uniqueID, functionPointer );
}

void RakClient::RegisterClassMemberRPC( char* uniqueID, void *functionPointer )
{
	RakPeer::RegisterClassMemberRPC( uniqueID, functionPointer );
}

void RakClient::SetTrackFrequencyTable( bool b )
{
	RakPeer::SetCompileFrequencyTable( b );
}

bool RakClient::GetSendFrequencyTable( unsigned int outputFrequencyTable[ 256 ] )
{
	return RakPeer::GetOutgoingFrequencyTable( outputFrequencyTable );
}

float RakClient::GetCompressionRatio( void ) const
{
	return RakPeer::GetCompressionRatio();
}

float RakClient::GetDecompressionRatio( void ) const
{
	return RakPeer::GetDecompressionRatio();
}

void RakClient::AttachPlugin( PluginInterface *messageHandler )
{
	RakPeer::AttachPlugin(messageHandler);
}

void RakClient::DetachPlugin( PluginInterface *messageHandler )
{
	RakPeer::DetachPlugin(messageHandler);
}

const char* RakClient::PlayerIDToDottedIP( const PlayerID playerId ) const
{
	return RakPeer::PlayerIDToDottedIP( playerId );
}

void RakClient::PushBackPacket( Packet *packet, bool pushAtHead )
{
	RakPeer::PushBackPacket(packet, pushAtHead);
}

bool RakClient::IsNetworkSimulatorActive( void )
{
	return RakPeer::IsNetworkSimulatorActive();
}

