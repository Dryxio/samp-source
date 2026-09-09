// Genuine vendor nonvirtual destruction with real accepted destructor providers.
#include "../raknet/RakNetworkFactory.h"
#include "../raknet/ReplicaManager.h"
#include "../raknet/TelnetTransport.h"
#include "../raknet/ConnectionGraph.h"

void RakNetworkFactory::DestroyReplicaManager(ReplicaManager* i)
{
    delete (ReplicaManager*) i;
}
void RakNetworkFactory::DestroyTelnetTransport(TelnetTransport* i)
{
    delete (TelnetTransport*) i;
}
void RakNetworkFactory::DestroyConnectionGraph(ConnectionGraph* i)
{
    delete (ConnectionGraph*) i;
}
