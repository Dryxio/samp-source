#include "../raknet/RakPeer.h"
#include "../raknet/SocketLayer.h"
#include "../raknet/GetTime.h"
#include "../raknet/PacketEnumerations.h"
#include "../raknet/PluginInterface.h"
#include "../raknet/RakAssert.h"
void RakPeer::Ping( const char* host, unsigned short remotePort, bool onlyReplyOnAcceptingConnections )
{
	if ( host == 0 )
		return;

//	if ( IsActive() == false )
//		return;

	// If the host starts with something other than 0, 1, or 2 it's (probably) a domain name.
	if ( host[ 0 ] < '0' || host[ 0 ] > '2' )
	{
#if !defined(_COMPATIBILITY_1)
		host = ( char* ) SocketLayer::Instance()->DomainNameToIP( host );
#else
		return;
#endif
	}

	PlayerID playerId;
	IPToPlayerID( host, remotePort, &playerId );

	RakNet::BitStream bitStream( sizeof(unsigned char) + sizeof(RakNetTime) );
	if ( onlyReplyOnAcceptingConnections )
		bitStream.Write((unsigned char)ID_PING_OPEN_CONNECTIONS);
	else
		bitStream.Write((unsigned char)ID_PING);

	bitStream.Write(RakNet::GetTime());

	unsigned i;
	for (i=0; i < messageHandlerList.Size(); i++)
		messageHandlerList[i]->OnDirectSocketSend((const char*)bitStream.GetData(), bitStream.GetNumberOfBitsUsed(), playerId);
	// No timestamp for 255.255.255.255
	SocketLayer::Instance()->SendTo( connectionSocket, (const char*)bitStream.GetData(), bitStream.GetNumberOfBytesUsed(), ( char* ) host, remotePort );




}
void RakPeer::AdvertiseSystem( const char *host, unsigned short remotePort, const char *data, int dataLength )
{
	if ( IsActive() == false )
		return ;

	if (host==0)
		return;

	// This is a security measure.  Don't send data longer than this value
	assert(dataLength <= MAX_OFFLINE_DATA_LENGTH);
	assert(dataLength>=0);

	// If the host starts with something other than 0, 1, or 2 it's (probably) a domain name.
	if ( host[ 0 ] < '0' || host[ 0 ] > '2' )
	{
#if !defined(_COMPATIBILITY_1)
		host = ( char* ) SocketLayer::Instance()->DomainNameToIP( host );
#else
		return;
#endif
	}

	PlayerID playerId;
	IPToPlayerID( host, remotePort, &playerId );

	RakNet::BitStream bitStream;
	bitStream.Write((unsigned char)ID_ADVERTISE_SYSTEM);
	if (dataLength>0)
		bitStream.Write(data, dataLength);
	else
		bitStream.Write((unsigned char)0); // Pad

	unsigned i;
	for (i=0; i < messageHandlerList.Size(); i++)
		messageHandlerList[i]->OnDirectSocketSend((const char*)bitStream.GetData(), bitStream.GetNumberOfBitsUsed(), playerId);
	SocketLayer::Instance()->SendTo( connectionSocket, (const char*)bitStream.GetData(), bitStream.GetNumberOfBytesUsed(), ( char* ) host, remotePort );



	/*
	// If the host starts with something other than 0, 1, or 2 it's (probably) a domain name.
	if ( host[ 0 ] < '0' || host[ 0 ] > '2' )
	{
#if !defined(_COMPATIBILITY_1)
		host = ( char* ) SocketLayer::Instance()->DomainNameToIP( host );
#else
		return;
#endif
	}

	PlayerID playerId;
	IPToPlayerID( host, remotePort, &playerId );

	RequestedConnectionStruct *rcs;
#ifdef _RAKNET_THREADSAFE
	rakPeerMutexes[requestedConnectionList_Mutex].Lock();
#endif
	rcs = requestedConnectionList.WriteLock();
	rcs->playerId=playerId;
	rcs->nextRequestTime=RakNet::GetTime();
	rcs->requestsMade=0;
	if (data && dataLength>0)
	{
		rcs->data=new char [dataLength];
		rcs->dataLength=(unsigned short)dataLength;
		memcpy(rcs->data, data, dataLength);
	}
	else
	{
		rcs->data=0;
		rcs->dataLength=0;
	}
	rcs->actionToTake=RequestedConnectionStruct::ADVERTISE_SYSTEM;
	requestedConnectionList.WriteUnlock();
#ifdef _RAKNET_THREADSAFE
	rakPeerMutexes[requestedConnectionList_Mutex].Unlock();
#endif
	*/
}
