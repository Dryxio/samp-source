#include "../raknet/NetworkTypes.h"

// Real source context: PlayerID bodies precede NetworkID bodies.
// Replaces the sole existing graph PlayerID owner after a successful probe.
bool NetworkID::peerToPeerMode=false;

bool PlayerID::operator==( const PlayerID& right ) const
{
	return binaryAddress == right.binaryAddress && port == right.port;
}

bool PlayerID::operator!=( const PlayerID& right ) const
{
	return binaryAddress != right.binaryAddress || port != right.port;
}

bool PlayerID::operator>( const PlayerID& right ) const
{
	return ( ( binaryAddress > right.binaryAddress ) || ( ( binaryAddress == right.binaryAddress ) && ( port > right.port ) ) );
}

bool PlayerID::operator<( const PlayerID& right ) const
{
	return ( ( binaryAddress < right.binaryAddress ) || ( ( binaryAddress == right.binaryAddress ) && ( port < right.port ) ) );
}

NetworkID& NetworkID::operator = ( const NetworkID& input )
{
	playerId = input.playerId;
	localSystemId = input.localSystemId;
	return *this;
}

bool NetworkID::operator==( const NetworkID& right ) const
{
	if (NetworkID::peerToPeerMode)
		return playerId == right.playerId && localSystemId == right.localSystemId;
	else
		return localSystemId==right.localSystemId;
}

bool NetworkID::operator!=( const NetworkID& right ) const
{
	if (NetworkID::peerToPeerMode)
		return playerId != right.playerId || localSystemId != right.localSystemId;
	else
		return localSystemId!=right.localSystemId;
}

bool NetworkID::operator>( const NetworkID& right ) const
{
	if (NetworkID::peerToPeerMode)
		return ( ( playerId > right.playerId ) || ( ( playerId == right.playerId ) && ( localSystemId > right.localSystemId ) ) );
	else
		return localSystemId>right.localSystemId;
}

bool NetworkID::operator<( const NetworkID& right ) const
{
	if (NetworkID::peerToPeerMode)
		return ( ( playerId < right.playerId ) || ( ( playerId == right.playerId ) && ( localSystemId < right.localSystemId ) ) );
	else
		return localSystemId<right.localSystemId;
}

