#include "../raknet/RakClient.h"

PlayerID RakClient::GetInternalID( void ) const
{
	return RakPeer::GetInternalID();
}
