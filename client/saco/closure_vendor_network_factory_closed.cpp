// Direct vendor factory methods whose real constructors already have accepted owners.
#include "../raknet/RakNetworkFactory.h"
#include "../raknet/ConsoleServer.h"
#include "../raknet/ReplicaManager.h"
#include "../raknet/LogCommandParser.h"
#include "../raknet/PacketLogger.h"
#include "../raknet/RakNetCommandParser.h"
#include "../raknet/TelnetTransport.h"
#include "../raknet/PacketConsoleLogger.h"
#include "../raknet/PacketFileLogger.h"
#include "../raknet/ConnectionGraph.h"

ConsoleServer* RakNetworkFactory::GetConsoleServer(void)
{
    return new ConsoleServer;
}
ReplicaManager* RakNetworkFactory::GetReplicaManager(void)
{
    return new ReplicaManager;
}
LogCommandParser* RakNetworkFactory::GetLogCommandParser(void)
{
    return new LogCommandParser;
}
PacketLogger* RakNetworkFactory::GetPacketLogger(void)
{
    return new PacketLogger;
}
RakNetCommandParser* RakNetworkFactory::GetRakNetCommandParser(void)
{
    return new RakNetCommandParser;
}
TelnetTransport* RakNetworkFactory::GetTelnetTransport(void)
{
    return new TelnetTransport;
}
PacketConsoleLogger* RakNetworkFactory::GetPacketConsoleLogger(void)
{
    return new PacketConsoleLogger;
}
PacketFileLogger* RakNetworkFactory::GetPacketFileLogger(void)
{
    return new PacketFileLogger;
}
ConnectionGraph* RakNetworkFactory::GetConnectionGraph(void)
{
    return new ConnectionGraph;
}
