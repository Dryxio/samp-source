// Complete vendor lifetime/reset methods; active packet-family providers are declared by header.
#include "../raknet/ReliabilityLayer.h"
#include "../raknet/GetTime.h"
#include "../raknet/SocketLayer.h"
#include "../raknet/PluginInterface.h"
#include "../raknet/RakAssert.h"
#include "../raknet/Rand.h"
#include "../raknet/PacketEnumerations.h"
#include <malloc.h>
static const float PING_MULTIPLIER_TO_RESEND=3.0;
static const RakNetTime MIN_PING_TO_RESEND=30;
static const int DEFAULT_HAS_RECEIVED_PACKET_QUEUE_SIZE=512;
__declspec(selectany) extern const double STARTING_SEND_BPS = 512000.0;
static const RakNetTimeNS TIME_TO_NEW_SAMPLE=500000;
static const RakNetTimeNS MAX_TIME_TO_SAMPLE=250000;
ReliabilityLayer::ReliabilityLayer() : updateBitStream( DEFAULT_MTU_SIZE )   // preallocate the update bitstream so we can avoid a lot of reallocs at runtime
{
#ifdef __USE_IO_COMPLETION_PORTS
	readWriteSocket = INVALID_SOCKET;
#endif

	freeThreadedMemoryOnNextUpdate = false;
#ifdef _DEBUG
	// Wait longer to disconnect in debug so I don't get disconnected while tracing
	timeoutTime=30000;
#else
	timeoutTime=10000;
#endif

#ifndef _RELEASE
	maxSendBPS=minExtraPing=extraPingVariance=0;
#endif

	InitializeVariables();
}

ReliabilityLayer::~ReliabilityLayer()
{
	FreeMemory( true ); // Free all memory immediately
#ifdef __USE_IO_COMPLETION_PORTS
	if ( readWriteSocket != INVALID_SOCKET )
		closesocket( readWriteSocket );
#endif
}

void ReliabilityLayer::Reset( bool resetVariables )
{
	FreeMemory( true ); // true because making a memory reset pending in the update cycle causes resets after reconnects.  Instead, just call Reset from a single thread
	if (resetVariables)
		InitializeVariables();
}

void ReliabilityLayer::InitializeVariables( void )
{
	memset( waitingForOrderedPacketReadIndex, 0, NUMBER_OF_ORDERED_STREAMS * sizeof(OrderingIndexType));
	memset( waitingForSequencedPacketReadIndex, 0, NUMBER_OF_ORDERED_STREAMS * sizeof(OrderingIndexType) );
	memset( waitingForOrderedPacketWriteIndex, 0, NUMBER_OF_ORDERED_STREAMS * sizeof(OrderingIndexType) );
	memset( waitingForSequencedPacketWriteIndex, 0, NUMBER_OF_ORDERED_STREAMS * sizeof(OrderingIndexType) );
	memset( &statistics, 0, sizeof( statistics ) );
	statistics.connectionStartTime = RakNet::GetTime();
	statistics.field_110 = RakNet::GetTime();
	statistics.field_114 = 0;
	statistics.field_118 = 0;
	statistics.field_11C = RakNet::GetTime();
	statistics.field_120 = 0;
	statistics.field_124 = 0;
	reliabilitySizeInBits = 4;
	splitPacketId = 0;
	messageNumber = 0;
	availableBandwidth=0;
	lastUpdateTime= RakNet::GetTimeNS();
	currentBandwidth=STARTING_SEND_BPS;
	// lastPacketSendTime=retransmittedFrames=sentPackets=sentFrames=receivedPacketsCount=bytesSent=bytesReceived=0;

	deadConnection = cheater = false;
	lastAckTime = 0;

	lowBandwidth=STARTING_SEND_BPS;
	histogramStartTime=lastUpdateTime+TIME_TO_NEW_SAMPLE+ping*2*1000;
	histogramEndTime=histogramStartTime+MAX_TIME_TO_SAMPLE;

	highBandwidth=0;
	histogramPlossCount=0;
	histogramAckCount=0;
	continuousSend=false;
	histogramReceiveMarker=0;
	noPacketlossIncreaseCount=0;
	nextAckTime=statistics.connectionStartTime;

	receivedPacketsBaseIndex=0;
	resetReceivedPackets=true;
	sendPacketCount=receivePacketCount=0;
	SetPing( 1000 );
	resendList.Preallocate(RESEND_TREE_ORDER*2);
}

void ReliabilityLayer::FreeMemory( bool freeAllImmediately )
{
	if ( freeAllImmediately )
	{
		FreeThreadedMemory();
		FreeThreadSafeMemory();
	}
	else
	{
		FreeThreadSafeMemory();
		freeThreadedMemoryOnNextUpdate = true;
	}
}

void ReliabilityLayer::FreeThreadedMemory( void )
{
}

void ReliabilityLayer::FreeThreadSafeMemory( void )
{
	unsigned i,j;
	InternalPacket *internalPacket;

	for (i=0; i < splitPacketChannelList.Size(); i++)
	{
		for (j=0; j < splitPacketChannelList[i]->splitPacketList.Size(); j++)
		{
			delete [] splitPacketChannelList[i]->splitPacketList[j]->data;
			internalPacketPool.ReleasePointer( splitPacketChannelList[i]->splitPacketList[j] );
		}
		delete splitPacketChannelList[i];
	}
	splitPacketChannelList.Clear();

	while ( outputQueue.Size() > 0 )
	{
		internalPacket = outputQueue.Pop();
		delete [] internalPacket->data;
		internalPacketPool.ReleasePointer( internalPacket );
	}

	outputQueue.ClearAndForceAllocation( 32 );

	for ( i = 0; i < orderingList.Size(); i++ )
	{
		if ( orderingList[ i ] )
		{
			DataStructures::LinkedList<InternalPacket*>* theList = orderingList[ i ];

			if ( theList )
			{
				while ( theList->Size() )
				{
					internalPacket = orderingList[ i ]->Pop();
					delete [] internalPacket->data;
					internalPacketPool.ReleasePointer( internalPacket );
				}

				delete theList;
			}
		}
	}

	orderingList.Clear();

	//resendList.ForEachData(DeleteInternalPacket);
	resendList.Clear();
	while ( resendQueue.Size() )
	{
		// The resend Queue can have NULL pointer holes.  This is so we can deallocate blocks without having to compress the array
		internalPacket = resendQueue.Pop();

		if ( internalPacket )
		{
			delete [] internalPacket->data;
			internalPacketPool.ReleasePointer( internalPacket );
		}
	}
	resendQueue.ClearAndForceAllocation( DEFAULT_HAS_RECEIVED_PACKET_QUEUE_SIZE );


	for ( i = 0; i < NUMBER_OF_PRIORITIES; i++ )
	{
		j = 0;
		for ( ; j < sendPacketSet[ i ].Size(); j++ )
		{
		delete [] ( sendPacketSet[ i ] ) [ j ]->data;
		internalPacketPool.ReleasePointer( ( sendPacketSet[ i ] ) [ j ] );
		}

		sendPacketSet[ i ].ClearAndForceAllocation( 32 ); // Preallocate the send lists so we don't do a bunch of reallocations unnecessarily
	}

#ifndef _RELEASE
	for (unsigned i = 0; i < delayList.Size(); i++ )
		delete delayList[ i ];
	delayList.Clear();
#endif

	internalPacketPool.ClearPool();

	//messageHistogram.Clear();

	acknowlegements.Clear();
}

typedef char R5ReliabilityLayerFullSize[(sizeof(ReliabilityLayer)==1794)?1:-1];


void ReliabilityLayer::SetPing( RakNetTime i )
{
	//assert(i < (RakNetTimeNS)timeoutTime*1000);
	if (i > timeoutTime)
		ping=500;
	else
		ping = i;
	if (ping < 30)
		ping=30; // Leave a buffer for variations in ping
#ifndef _RELEASE
	if (ping < (RakNetTime)(minExtraPing+extraPingVariance)*2)
		ping=(minExtraPing+extraPingVariance)*2;
#endif

	UpdateNextActionTime();
}

void ReliabilityLayer::UpdateNextActionTime(void)
{
	//double multiple = log10(currentBandwidth/MINIMUM_SEND_BPS) / 0.30102999566398119521373889472449;
	if (ping*(RakNetTime)PING_MULTIPLIER_TO_RESEND < MIN_PING_TO_RESEND)
		ackTimeIncrement=(RakNetTimeNS)MIN_PING_TO_RESEND*1000;
	else
		ackTimeIncrement=(RakNetTimeNS)(ping*(RakNetTime)PING_MULTIPLIER_TO_RESEND)*1000;
}
