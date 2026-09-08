// Explicit symbolic ABI adapters for genuinely implemented RakClient methods.
// Each complete adapter is independently checked against MSVC constructor-generated
// thunks, original interface slot targets, and accepted concrete callee providers.
// Unimplemented methods and the vendor constructor receive no coverage credit.
#include <string.h>
#include "../raknet/RakClient.h"
class R5RakClientInterfaceAdjustors { public:
Packet* Receive( void );
void DeallocatePacket( Packet *packet );
bool GenerateCompressionLayer( unsigned int inputFrequencyTable[ 256 ], bool inputLayer );
bool DeleteCompressionLayer( bool inputLayer );
void RegisterAsRemoteProcedureCall( char* uniqueID, void ( *functionPointer ) ( RPCParameters *rpcParms ) );
void RegisterClassMemberRPC( char* uniqueID, void *functionPointer );
float GetCompressionRatio( void ) const;
float GetDecompressionRatio( void ) const;
void AttachPlugin( PluginInterface *messageHandler );
void DetachPlugin( PluginInterface *messageHandler );
PlayerID GetInternalID( void ) const;
const char* PlayerIDToDottedIP( const PlayerID playerId ) const;
void PushBackPacket( Packet *packet, bool pushAtHead );
void ApplyNetworkSimulator( double maxSendBPS, unsigned short minExtraPing, unsigned short extraPingVariance);
bool IsNetworkSimulatorActive( void );
};
__declspec(naked) Packet* R5RakClientInterfaceAdjustors::Receive( void )
{
 __asm sub ecx, 0DDEh
 __asm jmp RakClient::Receive
}

__declspec(naked) void R5RakClientInterfaceAdjustors::DeallocatePacket( Packet *packet )
{
 __asm sub ecx, 0DDEh
 __asm jmp RakClient::DeallocatePacket
}

__declspec(naked) bool R5RakClientInterfaceAdjustors::GenerateCompressionLayer( unsigned int inputFrequencyTable[ 256 ], bool inputLayer )
{
 __asm sub ecx, 0DDEh
 __asm jmp RakClient::GenerateCompressionLayer
}

__declspec(naked) bool R5RakClientInterfaceAdjustors::DeleteCompressionLayer( bool inputLayer )
{
 __asm sub ecx, 0DDEh
 __asm jmp RakClient::DeleteCompressionLayer
}

__declspec(naked) void R5RakClientInterfaceAdjustors::RegisterAsRemoteProcedureCall( char* uniqueID, void ( *functionPointer ) ( RPCParameters *rpcParms ) )
{
 __asm sub ecx, 0DDEh
 __asm jmp RakClient::RegisterAsRemoteProcedureCall
}

__declspec(naked) void R5RakClientInterfaceAdjustors::RegisterClassMemberRPC( char* uniqueID, void *functionPointer )
{
 __asm sub ecx, 0DDEh
 __asm jmp RakClient::RegisterClassMemberRPC
}

__declspec(naked) float R5RakClientInterfaceAdjustors::GetCompressionRatio( void ) const
{
 __asm sub ecx, 0DDEh
 __asm jmp RakClient::GetCompressionRatio
}

__declspec(naked) float R5RakClientInterfaceAdjustors::GetDecompressionRatio( void ) const
{
 __asm sub ecx, 0DDEh
 __asm jmp RakClient::GetDecompressionRatio
}

__declspec(naked) void R5RakClientInterfaceAdjustors::AttachPlugin( PluginInterface *messageHandler )
{
 __asm sub ecx, 0DDEh
 __asm jmp RakClient::AttachPlugin
}

__declspec(naked) void R5RakClientInterfaceAdjustors::DetachPlugin( PluginInterface *messageHandler )
{
 __asm sub ecx, 0DDEh
 __asm jmp RakClient::DetachPlugin
}

__declspec(naked) PlayerID R5RakClientInterfaceAdjustors::GetInternalID( void ) const
{
 __asm sub ecx, 0DDEh
 __asm jmp RakClient::GetInternalID
}

__declspec(naked) const char* R5RakClientInterfaceAdjustors::PlayerIDToDottedIP( const PlayerID playerId ) const
{
 __asm sub ecx, 0DDEh
 __asm jmp RakClient::PlayerIDToDottedIP
}

__declspec(naked) void R5RakClientInterfaceAdjustors::PushBackPacket( Packet *packet, bool pushAtHead )
{
 __asm sub ecx, 0DDEh
 __asm jmp RakClient::PushBackPacket
}

__declspec(naked) void R5RakClientInterfaceAdjustors::ApplyNetworkSimulator( double maxSendBPS, unsigned short minExtraPing, unsigned short extraPingVariance)
{
 __asm sub ecx, 0DDEh
 __asm jmp RakClient::ApplyNetworkSimulator
}

__declspec(naked) bool R5RakClientInterfaceAdjustors::IsNetworkSimulatorActive( void )
{
 __asm sub ecx, 0DDEh
 __asm jmp RakClient::IsNetworkSimulatorActive
}
