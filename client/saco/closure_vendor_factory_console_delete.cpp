// Direct vendor factory destruction; typed ConsoleServer destructor independently identified.
#include "../raknet/RakNetworkFactory.h"
#include "../raknet/ConsoleServer.h"
void RakNetworkFactory::DestroyConsoleServer(ConsoleServer* i)
{
    delete (ConsoleServer*)i;
}
