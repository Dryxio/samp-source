#include <string.h>
// Bounded R5 RakClient forwarding wrappers.
//
// This unit intentionally excludes the constructor and the wrappers whose
// RakPeer targets are not yet accepted.  The definitions are the corresponding
// normal vendor/upstream/RakClient.cpp bodies.

#include "../raknet/RakClient.h"

void RakClient::UnregisterAsRemoteProcedureCall(char *uniqueID)
{
	RakPeer::UnregisterAsRemoteProcedureCall(uniqueID);
}

void RakClient::SetRouterInterface(RouterInterface *routerInterface)
{
	RakPeer::SetRouterInterface(routerInterface);
}

void RakClient::RemoveRouterInterface(RouterInterface *routerInterface)
{
	RakPeer::RemoveRouterInterface(routerInterface);
}

int RakClient::GetMTUSize(void) const
{
 return RakPeer::GetMTUSize();
}

void RakClient::AllowConnectionResponseIPMigration(bool allow)
{
	RakPeer::AllowConnectionResponseIPMigration(allow);
}

