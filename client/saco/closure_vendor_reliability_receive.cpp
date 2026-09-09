// R5 hypothesis: explicit null guards at packet data releases.
// Isolated header amalgamation: one nonvirtual R5 declaration gains bool* last parameter.
/// \file
/// \brief \b [Internal] Datagram reliable, ordered, unordered and sequenced sends.  Flow control.  Message splitting, reassembly, and coalescence.
///
/// This file is part of RakNet Copyright 2003 Kevin Jenkins.
///
/// Usage of RakNet is subject to the appropriate license agreement.
/// Creative Commons Licensees are subject to the
/// license found at
/// http://creativecommons.org/licenses/by-nc/2.5/
/// Single application licensees are subject to the license found at
/// http://www.rakkarsoft.com/SingleApplicationLicense.html
/// Custom license users are subject to the terms therein.
/// GPL license users are subject to the GNU General Public
/// License as published by the Free
/// Software Foundation; either version 2 of the License, or (at your
/// option) any later version.

#ifndef __RELIABILITY_LAYER_H
#define __RELIABILITY_LAYER_H

// SAMPSRV
#define TEA_ENCRYPTOR

#include "../raknet/SocketLayer.h"
#include "../raknet/MTUSize.h"
#include "../raknet/DS_LinkedList.h"
#include "../raknet/DS_List.h"
#include "../raknet/PacketPriority.h"
#include "../raknet/DS_Queue.h"
#include "../raknet/BitStream.h"
#include "../raknet/InternalPacket.h"
#include "../raknet/InternalPacketPool.h"

#ifdef TEA_ENCRYPTOR
#include "../raknet/TEABlockEncryptor.h"
#else
#include "../raknet/DataBlockEncryptor.h"
#endif

#include "../raknet/RakNetStatistics.h"
#include "../raknet/SHA1.h"
#include "../raknet/DS_OrderedList.h"
#include "../raknet/DS_RangeList.h"
#include "../raknet/DS_BPlusTree.h"

class PluginInterface;

/// Sizeof an UDP header in byte
#define UDP_HEADER_SIZE 28

/// Number of ordered streams available. You can use up to 32 ordered streams
#define NUMBER_OF_ORDERED_STREAMS 32 // 2^5

#define RESEND_TREE_ORDER 32

#include "../raknet/BitStream.h"

int SplitPacketIndexComp( SplitPacketIndexType const &key, InternalPacket* const &data );
struct SplitPacketChannel
{
	RakNetTimeNS lastUpdateTime;
	DataStructures::OrderedList<SplitPacketIndexType, InternalPacket*, SplitPacketIndexComp> splitPacketList;
};
int RAK_DLL_EXPORT SplitPacketChannelComp( SplitPacketIdType const &key, SplitPacketChannel* const &data );

/// Datagram reliable, ordered, unordered and sequenced sends.  Flow control.  Message splitting, reassembly, and coalescence.
class ReliabilityLayer
{
public:

	/// Constructor
	ReliabilityLayer();

	/// Destructor
	~ReliabilityLayer();

	/// Resets the layer for reuse
	void Reset( bool resetVariables );

	///Sets the encryption key.  Doing so will activate secure connections
	/// \param[in] key Byte stream for the encryption key
	void SetEncryptionKey( const unsigned char *key );

	/// Depreciated, from IO Completion ports
	/// \param[in] s The socket
	void SetSocket( SOCKET s );

	///	Returns what was passed to SetSocket
	/// \return The socket
	SOCKET GetSocket( void );

	/// Set the time, in MS, to use before considering ourselves disconnected after not being able to deliver a reliable packet
	/// Default time is 10,000 or 10 seconds in release and 30,000 or 30 seconds in debug.
	/// \param[in] time Time, in MS
	void SetTimeoutTime( RakNetTime time );

	/// Returns the value passed to SetTimeoutTime. or the default if it was never called
	/// \param[out] the value passed to SetTimeoutTime
	RakNetTime GetTimeoutTime(void);

	/// Packets are read directly from the socket layer and skip the reliability layer because unconnected players do not use the reliability layer
	/// This function takes packet data after a player has been confirmed as connected.
	/// \param[in] buffer The socket data
	/// \param[in] length The length of the socket data
	/// \param[in] playerId The player that this data is from
	/// \param[in] messageHandlerList A list of registered plugins
	/// \param[in] MTUSize maximum datagram size
	/// \retval true Success
	/// \retval false Modified packet
	bool HandleSocketReceiveFromConnectedPlayer( const char *buffer, int length, PlayerID playerId, DataStructures::List<PluginInterface*> &messageHandlerList, int MTUSize, bool *receiveStatus );

	/// This allocates bytes and writes a user-level message to those bytes.
	/// \param[out] data The message
	/// \return Returns number of BITS put into the buffer
	int Receive( unsigned char**data );

	/// Puts data on the send queue
	/// \param[in] data The data to send
	/// \param[in] numberOfBitsToSend The length of \a data in bits
	/// \param[in] priority The priority level for the send
	/// \param[in] reliability The reliability type for the send
	/// \param[in] orderingChannel 0 to 31.  Specifies what channel to use, for relational ordering and sequencing of packets.
	/// \param[in] makeDataCopy If true \a data will be copied.  Otherwise, only a pointer will be stored.
	/// \param[in] MTUSize maximum datagram size
	/// \param[in] currentTime Current time, as per RakNet::GetTime()
	/// \return True or false for success or failure.
	bool Send( char *data, int numberOfBitsToSend, PacketPriority priority, PacketReliability reliability, unsigned char orderingChannel, bool makeDataCopy, int MTUSize, RakNetTimeNS currentTime );

	/// Call once per game cycle.  Handles internal lists and actually does the send.
	/// \param[in] s the communication  end point
	/// \param[in] playerId The Unique Player Identifier who shouldhave sent some packets
	/// \param[in] MTUSize maximum datagram size
	/// \param[in] time current system time
	/// \param[in] messageHandlerList A list of registered plugins
	void Update(  SOCKET s, PlayerID playerId, int MTUSize, RakNetTimeNS time, DataStructures::List<PluginInterface*> &messageHandlerList );

	/// If Read returns -1 and this returns true then a modified packetwas detected
	/// \return true when a modified packet is detected
	bool IsCheater( void ) const;

	/// Were you ever unable to deliver a packet despite retries?
	/// \return true means the connection has been lost.  Otherwise not.
	bool IsDeadConnection( void ) const;

	/// Causes IsDeadConnection to return true
	void KillConnection(void);

	/// Sets the ping, which is used by the reliability layer to determine how long to wait for resends.  Mostly for flow control.
	/// \param[in] The ping time.
	void SetPing( RakNetTime i );

	/// Get Statistics
	/// \return A pointer to a static struct, filled out with current statistical information.
	RakNetStatisticsStruct * const GetStatistics( bool includeResendListDataSize );

	///Are we waiting for any data to be sent out or be processed by the player?
	bool IsDataWaiting(void);
	bool AreAcksWaiting(void);

	// Set outgoing lag and packet loss properties
	void ApplyNetworkSimulator( double _maxSendBPS, RakNetTime _minExtraPing, RakNetTime _extraPingVariance );

	/// Returns if you previously called ApplyNetworkSimulator
	/// \return If you previously called ApplyNetworkSimulator
	bool IsNetworkSimulatorActive( void );

	void SetSplitMessageProgressInterval(int interval);
	void SetUnreliableTimeout(RakNetTime timeoutMS);

private:

	/// Generates a datagram (coalesced packets)
	/// \param[out] output The resulting BitStream
	/// \param[in] Current MTU size
	/// \param[out] reliableDataSent Set to true or false as a return value as to if reliable data was sent.
	/// \param[in] time Current time
	/// \param[in] playerId Who we are sending to
	/// \param[in] messageHandlerList A list of registered plugins
	/// \return The number of messages sent
	unsigned GenerateDatagram( RakNet::BitStream *output, int MTUSize, bool *reliableDataSent, RakNetTimeNS time, PlayerID playerId, DataStructures::List<PluginInterface*> &messageHandlerList );

	/// Send the contents of a bitstream to the socket
	/// \param[in] s The socket used for sending data
	/// \param[in] playerId The address and port to send to
	/// \param[in] bitStream The data to send.
	void SendBitStream( SOCKET s, PlayerID playerId, RakNet::BitStream *bitStream );

	///Parse an internalPacket and create a bitstream to represent this dataReturns number of bits used
	int WriteToBitStreamFromInternalPacket( RakNet::BitStream *bitStream, const InternalPacket *const internalPacket );

	/// Parse a bitstream and create an internal packet to represent this data
	InternalPacket* CreateInternalPacketFromBitStream( RakNet::BitStream *bitStream, RakNetTimeNS time );

	/// Does what the function name says
	unsigned RemovePacketFromResendListAndDeleteOlderReliableSequenced( const MessageNumberType messageNumber, RakNetTimeNS time );

	/// Acknowledge receipt of the packet with the specified messageNumber
	void SendAcknowledgementPacket( const MessageNumberType messageNumber, RakNetTimeNS time );

	/// This will return true if we should not send at this time
	bool IsSendThrottled( int MTUSize );

	/// We lost a packet
	void UpdateWindowFromPacketloss( RakNetTimeNS time );

	/// Increase the window size
	void UpdateWindowFromAck( RakNetTimeNS time );

	/// Parse an internalPacket and figure out how many header bits would be written.  Returns that number
	int GetBitStreamHeaderLength( const InternalPacket *const internalPacket );

	/// Get the SHA1 code
	void GetSHA1( unsigned char * const buffer, unsigned int nbytes, char code[ SHA1_LENGTH ] );

	/// Check the SHA1 code
	bool CheckSHA1( char code[ SHA1_LENGTH ], unsigned char * const buffer, unsigned int nbytes );

	/// Search the specified list for sequenced packets on the specified ordering channel, optionally skipping those with splitPacketId, and delete them
	void DeleteSequencedPacketsInList( unsigned char orderingChannel, DataStructures::List<InternalPacket*>&theList, int splitPacketId = -1 );

	/// Search the specified list for sequenced packets with a value less than orderingIndex and delete them
	void DeleteSequencedPacketsInList( unsigned char orderingChannel, DataStructures::Queue<InternalPacket*>&theList );

	/// Returns true if newPacketOrderingIndex is older than the waitingForPacketOrderingIndex
	bool IsOlderOrderedPacket( OrderingIndexType newPacketOrderingIndex, OrderingIndexType waitingForPacketOrderingIndex );

	/// Split the passed packet into chunks under MTU_SIZE bytes (including headers) and save those new chunks
	void SplitPacket( InternalPacket *internalPacket, int MTUSize );

	/// Insert a packet into the split packet list
	void InsertIntoSplitPacketList( InternalPacket * internalPacket, RakNetTimeNS time );

	/// Take all split chunks with the specified splitPacketId and try to reconstruct a packet. If we can, allocate and return it.  Otherwise return 0
	InternalPacket * BuildPacketFromSplitPacketList( SplitPacketIdType splitPacketId, RakNetTimeNS time );

	/// Delete any unreliable split packets that have long since expired
	void DeleteOldUnreliableSplitPackets( RakNetTimeNS time );

	/// Creates a copy of the specified internal packet with data copied from the original starting at dataByteOffset for dataByteLength bytes.
	/// Does not copy any split data parameters as that information is always generated does not have any reason to be copied
	InternalPacket * CreateInternalPacketCopy( InternalPacket *original, int dataByteOffset, int dataByteLength, RakNetTimeNS time );

	/// Get the specified ordering list
	DataStructures::LinkedList<InternalPacket*> *GetOrderingListAtOrderingStream( unsigned char orderingChannel );

	/// Add the internal packet to the ordering list in order based on order index
	void AddToOrderingList( InternalPacket * internalPacket );

	/// Inserts a packet into the resend list in order
	void InsertPacketIntoResendList( InternalPacket *internalPacket, RakNetTimeNS time, bool makeCopyOfInternalPacket, bool firstResend );

	/// Memory handling
	void FreeMemory( bool freeAllImmediately );

	/// Memory handling
	void FreeThreadedMemory( void );

	/// Memory handling
	void FreeThreadSafeMemory( void );

	// Initialize the variables
	void InitializeVariables( void );

	/// Given the current time, is this time so old that we should consider it a timeout?
	bool IsExpiredTime(unsigned int input, RakNetTimeNS currentTime) const;

	// Make it so we don't do resends within a minimum threshold of time
	void UpdateNextActionTime(void);

	/// Does this packet number represent a packet that was skipped (out of order?)
	//unsigned int IsReceivedPacketHole(unsigned int input, RakNetTime currentTime) const;

	/// Skip an element in the received packets list
	//unsigned int MakeReceivedPacketHole(unsigned int input) const;

	/// How many elements are waiting to be resent?
	unsigned int GetResendListDataSize(void) const;

	/// Update all memory which is not threadsafe
	void UpdateThreadedMemory(void);

	void CalculateHistogramAckSize(void);

	DataStructures::List<DataStructures::LinkedList<InternalPacket*>*> orderingList;
	DataStructures::Queue<InternalPacket*> outputQueue;
	DataStructures::RangeList<MessageNumberType> acknowlegements;
	RakNetTimeNS nextAckTime;
	int splitMessageProgressInterval;
	RakNetTimeNS unreliableTimeout;
	
	DataStructures::BPlusTree<MessageNumberType, InternalPacket*, RESEND_TREE_ORDER> resendList;
	DataStructures::Queue<InternalPacket*> resendQueue;
	
	DataStructures::Queue<InternalPacket*> sendPacketSet[ NUMBER_OF_PRIORITIES ];
    DataStructures::OrderedList<SplitPacketIdType, SplitPacketChannel*, SplitPacketChannelComp> splitPacketChannelList;
	MessageNumberType messageNumber;
	//unsigned int windowSize;
	RakNetTimeNS lastAckTime;
	RakNet::BitStream updateBitStream;
	OrderingIndexType waitingForOrderedPacketWriteIndex[ NUMBER_OF_ORDERED_STREAMS ], waitingForSequencedPacketWriteIndex[ NUMBER_OF_ORDERED_STREAMS ];
	// Used for flow control (changed to regular TCP sliding window)
	// unsigned int maximumWindowSize, bytesSentSinceAck;
	// unsigned int outputWindowFullTime; // under linux if this last variable is on the line above it the delete operator crashes deleting this class!

	// STUFF TO NOT MUTEX HERE (called from non-conflicting threads, or value is not important)
	OrderingIndexType waitingForOrderedPacketReadIndex[ NUMBER_OF_ORDERED_STREAMS ], waitingForSequencedPacketReadIndex[ NUMBER_OF_ORDERED_STREAMS ];
	bool deadConnection, cheater;
	// unsigned int lastPacketSendTime,retransmittedFrames, sentPackets, sentFrames, receivedPacketsCount, bytesSent, bytesReceived,lastPacketReceivedTime;
	RakNetTime ping;
	SplitPacketIdType splitPacketId;
	RakNetTime timeoutTime; // How long to wait in MS before timing someone out
	//int MAX_AVERAGE_PACKETS_PER_SECOND; // Name says it all
//	int RECEIVED_PACKET_LOG_LENGTH, requestedReceivedPacketLogLength; // How big the receivedPackets array is
//	unsigned int *receivedPackets;
	unsigned int blockWindowIncreaseUntilTime;
	RakNetStatisticsStruct statistics;
	
	unsigned reliabilitySizeInBits;

	/// Memory-efficient receivedPackets algorithm:
	/// receivedPacketsBaseIndex is the packet number we are expecting
	/// Everything under receivedPacketsBaseIndex is a packet we already got
	/// Everything over receivedPacketsBaseIndex is stored in hasReceivedPacketQueue
	/// It stores the time to stop waiting for a particular packet number, where the packet number is receivedPacketsBaseIndex + the index into the queue
	/// If 0, we got got that packet.  Otherwise, the time to give up waiting for that packet.
	/// If we get a packet number where (receivedPacketsBaseIndex-packetNumber) is less than half the range of receivedPacketsBaseIndex then it is a duplicate
	/// Otherwise, it is a duplicate packet (and ignore it).
	DataStructures::Queue<RakNetTimeNS> hasReceivedPacketQueue;
	MessageNumberType receivedPacketsBaseIndex;
	bool resetReceivedPackets;

	RakNetTimeNS lastUpdateTime;

	RakNetTimeNS histogramEndTime, histogramStartTime;
	unsigned histogramReceiveMarker;
	int noPacketlossIncreaseCount;
	unsigned histogramPlossCount, histogramAckCount;
	double lowBandwidth, highBandwidth, currentBandwidth; // In bits per second
	double availableBandwidth;
	bool continuousSend;

#ifdef TEA_ENCRYPTOR
	TEABlockEncryptor encryptor;
#else
	DataBlockEncryptor encryptor;
#endif

	unsigned sendPacketCount, receivePacketCount;
	RakNetTimeNS ackTimeIncrement;

#ifdef __USE_IO_COMPLETION_PORTS
	///\note Windows Port only
	SOCKET readWriteSocket;
#endif
	///This variable is so that free memory can be called by only the update thread so we don't have to mutex things so much
	bool freeThreadedMemoryOnNextUpdate;

#ifndef _RELEASE
	struct DataAndTime
	{
		char data[ MAXIMUM_MTU_SIZE ];
		int length;
		RakNetTimeNS sendTime;
	};
	DataStructures::List<DataAndTime*> delayList;

	// Internet simulator
	double maxSendBPS;
	RakNetTime minExtraPing, extraPingVariance;
#endif

	// This has to be a member because it's not threadsafe when I removed the mutexes
	InternalPacketPool internalPacketPool;
};

#endif

// End isolated header amalgamation.
#include "../raknet/PluginInterface.h"
#include "../raknet/GetTime.h"
#include "../raknet/RakAssert.h"
#include <stddef.h>
static const int DEFAULT_HAS_RECEIVED_PACKET_QUEUE_SIZE=512;
bool ReliabilityLayer::HandleSocketReceiveFromConnectedPlayer( const char *buffer, int length, PlayerID playerId, DataStructures::List<PluginInterface*> &messageHandlerList, int MTUSize, bool *receiveStatus )
{
	*receiveStatus = false;
#ifdef _DEBUG
	assert( !( length <= 0 || buffer == 0 ) );
#endif

	if ( length <= 1 || buffer == 0 )   // Length of 1 is a connection request resend that we just ignore
		return true;

	//int numberOfAcksInFrame = 0;
	RakNetTimeNS time;
	bool indexFound;
	int count, size;
	MessageNumberType holeCount;
	unsigned i;
	unsigned ackedHistogramCounter;
	bool hasAcks=false;

//	bool duplicatePacket;

	// bytesReceived+=length + UDP_HEADER_SIZE;

	RakNetTime receiveTime = RakNet::GetTime();
	UpdateThreadedMemory();

	// decode this whole chunk if the decoder is defined.
	if ( encryptor.IsKeySet() )
	{
		if ( encryptor.Decrypt( ( unsigned char* ) buffer, length, ( unsigned char* ) buffer, &length ) == false )
		{
			statistics.bitsWithBadCRCReceived += length * 8;
			statistics.packetsWithBadCRCReceived++;
			return false;
		}
	}

	statistics.bitsReceived += length * 8;
	statistics.packetsReceived++;

	RakNet::BitStream socketData( (unsigned char*) buffer, length, false ); // Convert the incoming data to a bitstream for easy parsing
	time = RakNet::GetTimeNS();

	DataStructures::RangeList<MessageNumberType> incomingAcks;
	socketData.Read(hasAcks);
	if (hasAcks)
	{
		MessageNumberType messageNumber;
		if (incomingAcks.Deserialize(&socketData)==false)
			return false;
		for (i=0; i<incomingAcks.ranges.Size();i++)
		{
			if (incomingAcks.ranges[i].minIndex>incomingAcks.ranges[i].maxIndex)
			{
				RakAssert(incomingAcks.ranges[i].minIndex<=incomingAcks.ranges[i].maxIndex);
				return false;
			}

			statistics.acknowlegementsReceived += incomingAcks.ranges[i].maxIndex - incomingAcks.ranges[i].minIndex;
			for (messageNumber=incomingAcks.ranges[i].minIndex; messageNumber >= incomingAcks.ranges[i].minIndex && messageNumber <= incomingAcks.ranges[i].maxIndex; messageNumber++)
			{
				++statistics.field_120;
				hasAcks=true;

				// SHOW - ack received
				//printf("Got Ack for %i. resendList.Size()=%i sendQueue[0].Size() = %i\n",internalPacket->messageNumber, resendList.Size(), sendQueue[0].Size());
				ackedHistogramCounter=RemovePacketFromResendListAndDeleteOlderReliableSequenced( messageNumber, time );

#ifdef _DEBUG_LOGGER
				{
					char temp[256];
					sprintf(temp, "%p: Got ack for %i. Resend queue size=%i\n", this, messageNumber, resendQueue.Size());
					OutputDebugStr(temp);
				}
#endif

				if (time >= histogramStartTime && ackedHistogramCounter!=(unsigned)-1 && ackedHistogramCounter==histogramReceiveMarker)
					++histogramAckCount;

//				internalPacketPool.ReleasePointer( internalPacket );


				if ( resendList.IsEmpty() )
				{
					lastAckTime = 0; // Not resending anything so clear this var so we don't drop the connection on not getting any more acks
				}
				else
				{
					lastAckTime = time; // Just got an ack.  Record when we got it so we know the connection is alive
				}
			}
		}
	}


	// Parse the bitstream to create an internal packet
	InternalPacket* internalPacket = CreateInternalPacketFromBitStream( &socketData, time );

	if (internalPacket==0)
		return hasAcks;

	while ( internalPacket )
	{
		for (i=0; i < messageHandlerList.Size(); i++)
			messageHandlerList[i]->OnInternalPacket(internalPacket, receivePacketCount, playerId, (RakNetTime)(time/(RakNetTimeNS)1000), false);

		{
#ifdef _DEBUG_LOGGER
			{
				char temp[256];
				sprintf(temp, "%p: Got packet %i data: %i bitlen: %i\n", this, internalPacket->messageNumber, (unsigned char) internalPacket->data[0], internalPacket->dataBitLength);
				OutputDebugStr(temp);
			}
#endif

			//   receivedPacketsCount++;
			if ( internalPacket->reliability == RELIABLE_SEQUENCED || internalPacket->reliability == RELIABLE_ORDERED || internalPacket->reliability == RELIABLE )
			{
				SendAcknowledgementPacket( internalPacket->messageNumber, time );
			}

			// resetReceivedPackets is set from a non-threadsafe function.
			// We do the actual reset in this function so the data is not modified by multiple threads
			if (resetReceivedPackets)
			{
				hasReceivedPacketQueue.ClearAndForceAllocation(DEFAULT_HAS_RECEIVED_PACKET_QUEUE_SIZE);
				receivedPacketsBaseIndex=0;
				resetReceivedPackets=false;
			}

			// If the following conditional is true then this either a duplicate packet
			// or an older out of order packet
			// The subtraction unsigned overflow is intentional
			holeCount = (MessageNumberType)(internalPacket->messageNumber-receivedPacketsBaseIndex);
			const int typeRange = (MessageNumberType)-1;

			if (holeCount==0)
			{
				// Got what we were expecting
				if (hasReceivedPacketQueue.Size())
					hasReceivedPacketQueue.Pop();
				++receivedPacketsBaseIndex;
			}
			else if (holeCount > typeRange-typeRange/2)
			{
				// Underflow - got a packet we have already counted past
				statistics.duplicateMessagesReceived++;

				// Duplicate packet
				delete [] internalPacket->data;
				internalPacketPool.ReleasePointer( internalPacket );
				goto CONTINUE_SOCKET_DATA_PARSE_LOOP;
			}
			else if (holeCount<hasReceivedPacketQueue.Size())
			{
				// Got a higher count out of order packet that was missing in the sequence or we already got
				if (hasReceivedPacketQueue[holeCount]!=0) // non-zero means this is a hole
				{
					// Fill in the hole
					hasReceivedPacketQueue[holeCount]=0; // We got the packet at holeCount
				}
				else
				{
					// Not a hole - just a duplicate packet
					statistics.duplicateMessagesReceived++;

					// Duplicate packet
					delete [] internalPacket->data;
					internalPacketPool.ReleasePointer( internalPacket );
					goto CONTINUE_SOCKET_DATA_PARSE_LOOP;
				}
			}
			else // holeCount>=receivedPackets.Size()
			{
				// Got a higher count out of order packet whose messageNumber is higher than we have ever got

				// Add 0 times to the queue until (messageNumber - baseIndex) < queue size.
				while ((MessageNumberType)(holeCount) > hasReceivedPacketQueue.Size())
					hasReceivedPacketQueue.Push(time+(RakNetTimeNS)timeoutTime*1000); // Didn't get this packet - set the time to give up waiting
				hasReceivedPacketQueue.Push(0); // Got the packet
#ifdef _DEBUG
				// If this assert hits then MessageNumberType has overflowed
				assert(hasReceivedPacketQueue.Size() < (unsigned int)((MessageNumberType)(-1)));
#endif
			}

			// Pop all expired times.  0 means we got the packet, in which case we don't track this index either.
			while ( hasReceivedPacketQueue.Size()>0 && hasReceivedPacketQueue.Peek() < time )
			{
				hasReceivedPacketQueue.Pop();
				++receivedPacketsBaseIndex;
			}

			statistics.messagesReceived++;
			++statistics.field_114;
			if (receiveTime - statistics.field_110 > 1000) {
				statistics.field_110 = receiveTime;
				statistics.field_118 = statistics.field_114;
				statistics.field_114 = 0;
			}

			// If the allocated buffer is > DEFAULT_HAS_RECEIVED_PACKET_QUEUE_SIZE and it is 3x greater than the number of elements actually being used
			if (hasReceivedPacketQueue.AllocationSize() > (unsigned int) DEFAULT_HAS_RECEIVED_PACKET_QUEUE_SIZE && hasReceivedPacketQueue.AllocationSize() > hasReceivedPacketQueue.Size() * 3)
				hasReceivedPacketQueue.Compress();

			// Keep on top of deleting old unreliable split packets so they don't clog the list.
			if ( internalPacket->splitPacketCount > 0 )
				DeleteOldUnreliableSplitPackets( time );

			if ( internalPacket->reliability == RELIABLE_SEQUENCED || internalPacket->reliability == UNRELIABLE_SEQUENCED )
			{
#ifdef _DEBUG
				assert( internalPacket->orderingChannel < NUMBER_OF_ORDERED_STREAMS );
#endif

				if ( internalPacket->orderingChannel >= NUMBER_OF_ORDERED_STREAMS )
				{
					// Invalid packet
#ifdef _DEBUG
					printf( "Got invalid packet\n" );
#endif

					delete [] internalPacket->data;
					internalPacketPool.ReleasePointer( internalPacket );
					goto CONTINUE_SOCKET_DATA_PARSE_LOOP;
				}

				if ( IsOlderOrderedPacket( internalPacket->orderingIndex, waitingForSequencedPacketReadIndex[ internalPacket->orderingChannel ] ) == false )
				{
					statistics.sequencedMessagesInOrder++;


					// Is this a split packet?
					if ( internalPacket->splitPacketCount > 0 )
					{
						// Generate the split
						// Verify some parameters to make sure we don't get junk data


						// Check for a rebuilt packet
						InsertIntoSplitPacketList( internalPacket, time );

						// Sequenced
						internalPacket = BuildPacketFromSplitPacketList( internalPacket->splitPacketId, time );

						if ( internalPacket )
						{
							// Update our index to the newest packet
							waitingForSequencedPacketReadIndex[ internalPacket->orderingChannel ] = internalPacket->orderingIndex + 1;

							// If there is a rebuilt packet, add it to the output queue
							outputQueue.Push( internalPacket );
							internalPacket = 0;
						}

						// else don't have all the parts yet
					}

					else
					{
						// Update our index to the newest packet
						waitingForSequencedPacketReadIndex[ internalPacket->orderingChannel ] = internalPacket->orderingIndex + 1;

						// Not a split packet. Add the packet to the output queue
						outputQueue.Push( internalPacket );
						internalPacket = 0;
					}
				}
				else
				{
					statistics.sequencedMessagesOutOfOrder++;

					// Older sequenced packet. Discard it
					delete [] internalPacket->data;
					internalPacketPool.ReleasePointer( internalPacket );
				}

				goto CONTINUE_SOCKET_DATA_PARSE_LOOP;
			}

			// Is this an unsequenced split packet?
			if ( internalPacket->splitPacketCount > 0 )
			{
				// An unsequenced split packet.  May be ordered though.

				// Check for a rebuilt packet
				if ( internalPacket->reliability != RELIABLE_ORDERED )
					internalPacket->orderingChannel = 255; // Use 255 to designate not sequenced and not ordered

				InsertIntoSplitPacketList( internalPacket, time );

				internalPacket = BuildPacketFromSplitPacketList( internalPacket->splitPacketId, time );

				if ( internalPacket == 0 )
				{

					// Don't have all the parts yet
					goto CONTINUE_SOCKET_DATA_PARSE_LOOP;
				}

				// else continue down to handle RELIABLE_ORDERED
			}

			if ( internalPacket->reliability == RELIABLE_ORDERED )
			{
#ifdef _DEBUG
				assert( internalPacket->orderingChannel < NUMBER_OF_ORDERED_STREAMS );
#endif

				if ( internalPacket->orderingChannel >= NUMBER_OF_ORDERED_STREAMS )
				{
#ifdef _DEBUG
					printf("Got invalid ordering channel %i from packet %i\n", internalPacket->orderingChannel, internalPacket->messageNumber);
#endif
					// Invalid packet
					delete [] internalPacket->data;
					internalPacketPool.ReleasePointer( internalPacket );
					goto CONTINUE_SOCKET_DATA_PARSE_LOOP;
				}

				if ( waitingForOrderedPacketReadIndex[ internalPacket->orderingChannel ] == internalPacket->orderingIndex )
				{
					// Get the list to hold ordered packets for this stream
					DataStructures::LinkedList<InternalPacket*> *orderingListAtOrderingStream;
					unsigned char orderingChannelCopy = internalPacket->orderingChannel;

					statistics.orderedMessagesInOrder++;

					// Show ordering index increment
					//printf("Pushing immediate packet %i with ordering index %i\n", internalPacket->messageNumber, internalPacket->orderingIndex );

					// Push the packet for the user to read
					outputQueue.Push( internalPacket );
					internalPacket = 0; // Don't reference this any longer since other threads access it

					// Wait for the next ordered packet in sequence
					waitingForOrderedPacketReadIndex[ orderingChannelCopy ] ++; // This wraps

					orderingListAtOrderingStream = GetOrderingListAtOrderingStream( orderingChannelCopy );

					if ( orderingListAtOrderingStream != 0)
					{
						while ( orderingListAtOrderingStream->Size() > 0 )
						{
							// Cycle through the list until nothing is found
							orderingListAtOrderingStream->Beginning();
							indexFound=false;
							size=orderingListAtOrderingStream->Size();
							count=0;

							while (count++ < size)
							{
								if ( orderingListAtOrderingStream->Peek()->orderingIndex == waitingForOrderedPacketReadIndex[ orderingChannelCopy ] )
								{
									/*
									RakNet::BitStream temp(orderingListAtOrderingStream->Peek()->data, BITS_TO_BYTES(orderingListAtOrderingStream->Peek()->dataBitLength), false);
									temp.IgnoreBits(8);
									unsigned int receivedPacketNumber=0;
									temp.Read(receivedPacketNumber);
									printf("Receive: receivedPacketNumber=%i orderingIndex=%i waitingFor=%i\n", receivedPacketNumber, orderingListAtOrderingStream->Peek()->orderingIndex, waitingForOrderedPacketReadIndex[ orderingChannelCopy ]);
									*/

									//printf("Pushing delayed packet %i with ordering index %i. outputQueue.Size()==%i\n", orderingListAtOrderingStream->Peek()->messageNumber, orderingListAtOrderingStream->Peek()->orderingIndex, outputQueue.Size() );
									outputQueue.Push( orderingListAtOrderingStream->Pop() );
									waitingForOrderedPacketReadIndex[ orderingChannelCopy ]++; // This wraps at 255
									indexFound=true;
								}
								else
									(*orderingListAtOrderingStream)++;
							}

							if (indexFound==false)
								break;
						}
					}

					internalPacket = 0;
				}
				else
				{
				//	assert(waitingForOrderedPacketReadIndex[ internalPacket->orderingChannel ] < internalPacket->orderingIndex);
					statistics.orderedMessagesOutOfOrder++;

					// This is a newer ordered packet than we are waiting for. Store it for future use
					AddToOrderingList( internalPacket );
				}

				goto CONTINUE_SOCKET_DATA_PARSE_LOOP;
			}

			// Nothing special about this packet.  Add it to the output queue
			outputQueue.Push( internalPacket );

			internalPacket = 0;
		}

		// Used for a goto to jump to the next packet immediately

	CONTINUE_SOCKET_DATA_PARSE_LOOP:
		// Parse the bitstream to create an internal packet
		internalPacket = CreateInternalPacketFromBitStream( &socketData, time );
	}

	/*
	if (numberOfAcksInFrame > 0)
//		if (time > lastWindowAdjustTime+ping)
	{
	//	printf("Window size up\n");
		windowSize+=1 + numberOfAcksInFrame/windowSize;
		if ( windowSize > MAXIMUM_WINDOW_SIZE )
			windowSize = MAXIMUM_WINDOW_SIZE;
		//lastWindowAdjustTime=time;
	}
	//else
	//	printf("No acks in frame\n");
*/
	/*
	// numberOfAcksInFrame>=windowSize means that all the packets we last sent from the resendList are cleared out
	// 11/17/05 - the problem with numberOfAcksInFrame >= windowSize is that if the entire frame is filled with resends but not all resends filled the frame
	// then the sender is limited by how many resends can fit in one frame
	if ( numberOfAcksInFrame >= windowSize && ( sendPacketSet[ SYSTEM_PRIORITY ].Size() > 0 || sendPacketSet[ HIGH_PRIORITY ].Size() > 0 || sendPacketSet[ MEDIUM_PRIORITY ].Size() > 0 ) )
	{
		// reliabilityLayerMutexes[windowSize_MUTEX].Lock();
		//printf("windowSize=%i lossyWindowSize=%i\n", windowSize, lossyWindowSize);

		if ( windowSize < lossyWindowSize || (time>lastWindowIncreaseSizeTime && time-lastWindowIncreaseSizeTime>lostPacketResendDelay*2) )   // Increases the window size slowly, testing for packetloss
		{
			// If we get a frame which clears out the resend queue after handling one or more acks, and we have packets waiting to go out,
			// and we didn't recently lose a packet then increase the window size by 1
			windowSize++;

			if ( (time>lastWindowIncreaseSizeTime && time-lastWindowIncreaseSizeTime>lostPacketResendDelay*2) )   // The increase is to test for packetloss
				lastWindowIncreaseSizeTime = time;

			// If the window is so large that we couldn't possibly fit any more packets into the frame, then just leave it alone
			if ( windowSize > MAXIMUM_WINDOW_SIZE )
				windowSize = MAXIMUM_WINDOW_SIZE;

			// SHOW - WINDOWING
			//else
			//	printf("Increasing windowSize to %i.  Lossy window size = %i\n", windowSize, lossyWindowSize);

			// If we are more than 5 over the lossy window size, increase the lossy window size by 1
			if ( windowSize == MAXIMUM_WINDOW_SIZE || windowSize - lossyWindowSize > 5 )
				lossyWindowSize++;
		}
		// reliabilityLayerMutexes[windowSize_MUTEX].Unlock();
	}
	*/

	if (hasAcks)
	{
		UpdateWindowFromAck(time);
	}

	receivePacketCount++;

	return true;
}

void ReliabilityLayer::UpdateWindowFromAck( RakNetTimeNS time )
{

}

bool ReliabilityLayer::IsOlderOrderedPacket( OrderingIndexType newPacketOrderingIndex, OrderingIndexType waitingForPacketOrderingIndex )
{
	// This should give me 255 or 65535
	OrderingIndexType maxRange = (OrderingIndexType) -1;

	if ( waitingForPacketOrderingIndex > maxRange/2 )
	{
		if ( newPacketOrderingIndex >= waitingForPacketOrderingIndex - maxRange/2+1 && newPacketOrderingIndex < waitingForPacketOrderingIndex )
		{
			return true;
		}
	}

	else
		if ( newPacketOrderingIndex >= ( OrderingIndexType ) ( waitingForPacketOrderingIndex - (( OrderingIndexType ) maxRange/2+1) ) ||
			newPacketOrderingIndex < waitingForPacketOrderingIndex )
		{
			return true;
		}

	// Old packet
	return false;
}

void ReliabilityLayer::UpdateThreadedMemory(void)
{
	if ( freeThreadedMemoryOnNextUpdate )
	{
		freeThreadedMemoryOnNextUpdate = false;
		// The vendor FreeThreadedMemory body is empty; no operation to reconstruct.
	}
}

