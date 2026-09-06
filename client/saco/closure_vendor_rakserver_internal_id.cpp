#include "../raknet/RakServer.h"

PlayerID RakServer::GetInternalID( void ) const
{
	return RakPeer::GetInternalID();
}
