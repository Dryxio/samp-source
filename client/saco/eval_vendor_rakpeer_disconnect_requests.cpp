// Complete original RakPeer disconnect/request lifecycle methods, source order preserved.
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "../raknet/RakNetDefines.h"
#include "../raknet/RakPeer.h"
#include "../raknet/GetTime.h"
#include "../raknet/PacketEnumerations.h"
#include "../raknet/PluginInterface.h"
#include "../raknet/RakSleep.h"
#include "../raknet/RakAssert.h"

void RakPeer::Disconnect( unsigned int blockDuration, unsigned char orderingChannel )
{
	unsigned i,j;
	bool anyActive;
	RakNetTime startWaitingTime;
//	PlayerID playerId;
	RakNetTime time;
	//unsigned short systemListSize = remoteSystemListSize; // This is done for threading reasons
	unsigned short systemListSize = maximumNumberOfPeers;

	if ( blockDuration > 0 )
	{
		for ( i = 0; i < systemListSize; i++ )
		{
			// remoteSystemList in user thread
			NotifyAndFlagForDisconnect(remoteSystemList[i].playerId, false, orderingChannel);
		}

		time = RakNet::GetTime();
		startWaitingTime = time;
		while ( time - startWaitingTime < blockDuration )
		{
			anyActive=false;
			for (j=0; j < systemListSize; j++)
			{
				// remoteSystemList in user thread
				if (remoteSystemList[j].isActive)
				{
					anyActive=true;
					break;
				}
			}

			// If this system is out of packets to send, then stop waiting
			if ( anyActive==false )
				break;

			// This will probably cause the update thread to run which will probably
			// send the disconnection notification

			RakSleep(15);
			time = RakNet::GetTime();
		}
	}

	for (i=0; i < messageHandlerList.Size(); i++)
	{
		messageHandlerList[i]->OnDisconnect(this);
	}

	if ( endThreads == false )
	{
		// Stop the threads
		endThreads = true;

		// Normally the thread will call DecreaseUserCount on termination but if we aren't using threads just do it
		// manually
#ifdef __USE_IO_COMPLETION_PORTS
		AsynchronousFileIO::Instance()->DecreaseUserCount();
#endif
	}

	while ( isMainLoopThreadActive )
		RakSleep(15);

	// remoteSystemList in Single thread
	for ( i = 0; i < systemListSize; i++ )
	{
		// Reserve this reliability layer for ourselves
		remoteSystemList[ i ].isActive = false;

		// Remove any remaining packets
		remoteSystemList[ i ].reliabilityLayer.Reset(false);
	}

	// Clear the lookup table.  Safe to call from the user thread since the network thread is now stopped
	remoteSystemLookup.Clear();

	// Setting maximumNumberOfPeers to 0 allows remoteSystemList to be reallocated in Initialize.
	// Setting remoteSystemListSize prevents threads from accessing the reliability layer
	maximumNumberOfPeers = 0;
	//remoteSystemListSize = 0;

	// Free any packets the user didn't deallocate
	Packet **packet;
#ifdef _RAKNET_THREADSAFE
	rakPeerMutexes[transferToPacketQueue_Mutex].Lock();
#endif
	packet=packetSingleProducerConsumer.ReadLock();
	while (packet)
	{
		DeallocatePacket(*packet);
		packetSingleProducerConsumer.ReadUnlock();
		packet=packetSingleProducerConsumer.ReadLock();
	}
	packetSingleProducerConsumer.Clear();
#ifdef _RAKNET_THREADSAFE
	rakPeerMutexes[transferToPacketQueue_Mutex].Unlock();
#endif

#ifdef _RAKNET_THREADSAFE
	rakPeerMutexes[packetPool_Mutex].Lock();
#endif
	for (i=0; i < packetPool.Size(); i++)
		DeallocatePacket(packetPool[i]);
	packetPool.Clear();
#ifdef _RAKNET_THREADSAFE
	rakPeerMutexes[packetPool_Mutex].Unlock();
#endif

	blockOnRPCReply=false;

	if ( connectionSocket != INVALID_SOCKET )
	{
		closesocket( connectionSocket );
		connectionSocket = INVALID_SOCKET;
	}

	ClearBufferedCommands();
	bytesSentPerSecond = bytesReceivedPerSecond = 0;

	ClearRequestedConnectionList();

#if defined (_WIN32) && defined(USE_WAIT_FOR_MULTIPLE_EVENTS)
	if (recvEvent!=INVALID_HANDLE_VALUE)
	{
		CloseHandle( recvEvent );
		recvEvent = INVALID_HANDLE_VALUE;
	}	
#endif

	// Clear out the reliability layer list in case we want to reallocate it in a successive call to Init.
	RemoteSystemStruct * temp = remoteSystemList;
	remoteSystemList = 0;
	delete [] temp;
}

RakNet::BitStream * RakPeer::GetRemoteStaticData( const PlayerID playerId )
{
	if ( playerId == myPlayerId )
		return & localStaticData;

	RemoteSystemStruct *remoteSystem = GetRemoteSystemFromPlayerID( playerId, false, false );

	if ( remoteSystem )
		return &(remoteSystem->staticData);
	else
		return 0;
}

void RakPeer::SetRemoteStaticData( const PlayerID playerId, const char *data, const int length )
{
	if ( playerId == myPlayerId )
	{
		localStaticData.Reset();

		if ( data && length > 0 )
			localStaticData.Write( data, length );
	}
	else
	{
		RemoteSystemStruct *remoteSystem = GetRemoteSystemFromPlayerID( playerId, false, true );

		if ( remoteSystem == 0 )
			return;

		remoteSystem->staticData.Reset();
		remoteSystem->staticData.Write( data, length );
	}
}

RPCMap* RakPeer::GetRPCMap( const PlayerID playerId)
{
    if (playerId==UNASSIGNED_PLAYER_ID)
		return &rpcMap;
	else
	{
		RemoteSystemStruct *rss=GetRemoteSystemFromPlayerID(playerId, false, true);
		if (rss)
			return &(rss->rpcMap);
		else
			return 0;
	}
}

bool RakPeer::SendConnectionRequest( const char* host, unsigned short remotePort, char* passwordData, int passwordDataLength )
{
	PlayerID playerId;
	IPToPlayerID( host, remotePort, &playerId );

	// Already connected?
	if (GetRemoteSystemFromPlayerID(playerId, false, true))
		return false;

	assert(passwordDataLength <= 256);

#ifdef _RAKNET_THREADSAFE
	rakPeerMutexes[requestedConnectionList_Mutex].Lock();
#endif
	RequestedConnectionStruct *rcs = requestedConnectionList.WriteLock();
	rcs->playerId=playerId;
	rcs->nextRequestTime=RakNet::GetTime();
	rcs->requestsMade=0;
	rcs->data=0;
	rcs->actionToTake=RequestedConnectionStruct::CONNECT;
	memcpy(rcs->outgoingPassword, passwordData, passwordDataLength);
	rcs->outgoingPasswordLength=(unsigned char) passwordDataLength;
	requestedConnectionList.WriteUnlock();

#ifdef _RAKNET_THREADSAFE
	rakPeerMutexes[requestedConnectionList_Mutex].Unlock();
#endif

	return true;
}

void RakPeer::IPToPlayerID( const char* host, unsigned short remotePort, PlayerID *playerId )
{
	if ( host == 0 )
		return ;

	playerId->binaryAddress = inet_addr( host );

	playerId->port = remotePort;
}

void RakPeer::ClearBufferedCommands(void)
{
	BufferedCommandStruct *bcs;

#ifdef _RAKNET_THREADSAFE
	rakPeerMutexes[bufferedCommands_Mutex].Lock();
#endif
	while ((bcs=bufferedCommands.ReadLock())!=0)
	{
		if (bcs->data)
			delete [] bcs->data;

        bufferedCommands.ReadUnlock();
	}
	bufferedCommands.Clear();
#ifdef _RAKNET_THREADSAFE
	rakPeerMutexes[bufferedCommands_Mutex].Unlock();
#endif
}

void RakPeer::ClearRequestedConnectionList(void)
{
	RequestedConnectionStruct *bcs;
#ifdef _RAKNET_THREADSAFE
	rakPeerMutexes[requestedConnectionList_Mutex].Lock();
#endif
	while ((bcs=requestedConnectionList.ReadLock())!=0)
	{
		if (bcs->data)
			delete [] bcs->data;

		requestedConnectionList.ReadUnlock();
	}
	requestedConnectionList.Clear();
#ifdef _RAKNET_THREADSAFE
	rakPeerMutexes[requestedConnectionList_Mutex].Unlock();
#endif
}
