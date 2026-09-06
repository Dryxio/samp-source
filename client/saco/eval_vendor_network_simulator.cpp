// Original vendor network simulation settings, no network execution.
#include "../raknet/ReliabilityLayer.h"
#include "../raknet/RakPeer.h"
#include "../raknet/RakClient.h"
#include "../raknet/RakServer.h"

void ReliabilityLayer::ApplyNetworkSimulator( double _maxSendBPS, RakNetTime _minExtraPing, RakNetTime _extraPingVariance )
{
#ifndef _RELEASE
	maxSendBPS=_maxSendBPS;
	minExtraPing=_minExtraPing;
	extraPingVariance=_extraPingVariance;
	if (ping < (unsigned int)(minExtraPing+extraPingVariance)*2)
		ping=(minExtraPing+extraPingVariance)*2;
#endif
}

void RakPeer::ApplyNetworkSimulator( double maxSendBPS, unsigned short minExtraPing, unsigned short extraPingVariance)
{
#ifndef _RELEASE
	if (remoteSystemList)
	{
		unsigned short i;
		for (i=0; i < maximumNumberOfPeers; i++)
		//for (i=0; i < remoteSystemListSize; i++)
			remoteSystemList[i].reliabilityLayer.ApplyNetworkSimulator(maxSendBPS, minExtraPing, extraPingVariance);
	}

	_maxSendBPS=maxSendBPS;
	_minExtraPing=minExtraPing;
	_extraPingVariance=extraPingVariance;
#endif
}

bool RakPeer::IsNetworkSimulatorActive( void )
{
	return _maxSendBPS>0 || _minExtraPing>0 || _extraPingVariance>0;
}

void RakClient::ApplyNetworkSimulator( double maxSendBPS, unsigned short minExtraPing, unsigned short extraPingVariance)
{
	RakPeer::ApplyNetworkSimulator( maxSendBPS, minExtraPing, extraPingVariance );
}

void RakServer::ApplyNetworkSimulator( double maxSendBPS, unsigned short minExtraPing, unsigned short extraPingVariance)
{
	RakPeer::ApplyNetworkSimulator( maxSendBPS, minExtraPing, extraPingVariance );
}
