#include <string.h>
#include "../raknet/RakPeer.h"
// Genuine vendor leaf providers; coverage already accepted historically is deduplicated.
void RakPeer::UnregisterAsRemoteProcedureCall(char *uniqueID) { }
void RakPeer::SetRouterInterface(RouterInterface *routerInterface) { router=routerInterface; }
void RakPeer::RemoveRouterInterface(RouterInterface *routerInterface) { if(router==routerInterface) router=0; }
void RakPeer::AllowConnectionResponseIPMigration(bool allow) { allowConnectionResponseIPMigration=allow; }
int RakPeer::GetMTUSize(void) const { return MTUSize; }
