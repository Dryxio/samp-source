#include "../raknet/NetworkTypes.h"

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
