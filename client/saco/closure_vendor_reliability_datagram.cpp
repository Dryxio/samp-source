// R5 adaptation: explicit data-null guards at all three packet buffer releases.
#include "../raknet/ReliabilityLayer.h"
#include "../raknet/PluginInterface.h"
#include "../raknet/RakAssert.h"
#include <stddef.h>
static const float PING_MULTIPLIER_TO_RESEND=3.0f;

unsigned ReliabilityLayer::GenerateDatagram( RakNet::BitStream *output, int MTUSize, bool *reliableDataSent, RakNetTimeNS time, PlayerID playerId, DataStructures::List<PluginInterface*> &messageHandlerList  )
{
	InternalPacket * internalPacket;
//	InternalPacket *temp;
	int maxDataBitSize;
	int reliableBits = 0;
	int nextPacketBitLength;
	unsigned i, messageHandlerIndex;
	bool isReliable, onlySendUnreliable;
	bool writeFalseToHeader;
	unsigned messagesSent=0;

	maxDataBitSize = MTUSize - UDP_HEADER_SIZE;

	if ( encryptor.IsKeySet() )
		maxDataBitSize -= 8; // Extra data for the encryptor

	maxDataBitSize <<= 3;

	*reliableDataSent = false;

	if (time > nextAckTime)
	{
		if (acknowlegements.Size()>0)
		{
			output->Write(true);
			messagesSent++;
			statistics.acknowlegementBitsSent +=acknowlegements.Serialize(output, (MTUSize-UDP_HEADER_SIZE)*8-1, true);
			if (acknowlegements.Size()==0)
				nextAckTime=time+(RakNetTimeNS)(ping*(RakNetTime)(PING_MULTIPLIER_TO_RESEND/4.0f));
			else
			{
			//	printf("Ack full\n");
			}

			writeFalseToHeader=false;
		}
		else
		{
			writeFalseToHeader=true;
			nextAckTime=time+(RakNetTimeNS)(ping*(RakNetTime)(PING_MULTIPLIER_TO_RESEND/4.0f));
		}
	}
	else
		writeFalseToHeader=true;

	while ( resendQueue.Size() > 0 )
	{
		internalPacket = resendQueue.Peek();
		// The resend Queue can have holes.  This is so we can deallocate blocks without having to compress the array
		if ( internalPacket->nextActionTime == 0 )
		{
			resendQueue.Pop();
			if (internalPacket->data)
				delete [] internalPacket->data;
			internalPacketPool.ReleasePointer( internalPacket );
			continue; // This was a hole
		}

		if ( resendQueue.Peek()->nextActionTime < time )
		{
			internalPacket = resendQueue.Pop();

			nextPacketBitLength = GetBitStreamHeaderLength( internalPacket ) + internalPacket->dataBitLength;

			if ( output->GetNumberOfBitsUsed() + nextPacketBitLength > maxDataBitSize )
			{
				resendQueue.PushAtHead( internalPacket ); // Not enough room to use this packet after all!

				goto END_OF_GENERATE_FRAME;
			}

			RakAssert(internalPacket->priority >= 0);

#ifdef _DEBUG_LOGGER
			{
				char temp[256];
				sprintf(temp, "%p: Resending packet %i data: %i bitlen: %i\n", this, internalPacket->messageNumber, (unsigned char) internalPacket->data[0], internalPacket->dataBitLength);
				OutputDebugStr(temp);
			}
#endif

			for (messageHandlerIndex=0; messageHandlerIndex < messageHandlerList.Size(); messageHandlerIndex++)
				messageHandlerList[messageHandlerIndex]->OnInternalPacket(internalPacket, sendPacketCount, playerId, (RakNetTime)(time/(RakNetTimeNS)1000), true);

			// Write to the output bitstream
			statistics.messageResends++;
			statistics.messageDataBitsResent += internalPacket->dataBitLength;

			if (writeFalseToHeader)
			{
				output->Write(false);
				writeFalseToHeader=false;
			}
			statistics.messagesTotalBitsResent += WriteToBitStreamFromInternalPacket( output, internalPacket );
			internalPacket->packetNumber=sendPacketCount;
			messagesSent++;

			*reliableDataSent = true;

			statistics.packetsContainingOnlyAcknowlegementsAndResends++;

			internalPacket->nextActionTime = time + ackTimeIncrement;
			if (time >= histogramStartTime && internalPacket->histogramMarker==histogramReceiveMarker)
				histogramPlossCount++;

			internalPacket->histogramMarker=histogramReceiveMarker;

			//printf("PACKETLOSS\n ");

			// Put the packet back into the resend list at the correct spot
			// Don't make a copy since I'm reinserting an allocated struct
			InsertPacketIntoResendList( internalPacket, time, false, false );

		}
		else
		{
			break;
		}
	}


	onlySendUnreliable = false;


	// From highest to lowest priority, fill up the output bitstream from the send lists
	for ( i = 0; i < NUMBER_OF_PRIORITIES; i++ )
	{
		while ( sendPacketSet[ i ].Size() )
		{
			internalPacket = sendPacketSet[ i ].Pop();

			nextPacketBitLength = GetBitStreamHeaderLength( internalPacket ) + internalPacket->dataBitLength;

			if (unreliableTimeout!=0 &&
				(internalPacket->reliability==UNRELIABLE || internalPacket->reliability==UNRELIABLE_SEQUENCED) &&
				time > internalPacket->creationTime+(RakNetTimeNS)unreliableTimeout)
			{
				// Unreliable packets are deleted
				if (internalPacket->data)
				delete [] internalPacket->data;
				internalPacketPool.ReleasePointer( internalPacket );
				continue;
			}


			if ( output->GetNumberOfBitsUsed() + nextPacketBitLength > maxDataBitSize )
			{
				// This output won't fit.
				sendPacketSet[ i ].PushAtHead( internalPacket ); // Push this back at the head so it is the next thing to go out
				break;
			}

			if ( internalPacket->reliability == RELIABLE || internalPacket->reliability == RELIABLE_SEQUENCED || internalPacket->reliability == RELIABLE_ORDERED )
				isReliable = true;
			else
				isReliable = false;

			// Write to the output bitstream
			statistics.messagesSent[ i ] ++;
			// R5 stores four uint32 reliable-message counters in the existing 16-byte field.
			if (isReliable)
				((unsigned *)statistics._pad20)[i]++;
			statistics.messageDataBitsSent[ i ] += internalPacket->dataBitLength;

#ifdef _DEBUG_LOGGER
			{
				char temp[256];
				sprintf(temp, "%p: Sending packet %i data: %i bitlen: %i\n", this, internalPacket->messageNumber, (unsigned char) internalPacket->data[0], internalPacket->dataBitLength);
				OutputDebugStr(temp);
			}
#endif

			for (messageHandlerIndex=0; messageHandlerIndex < messageHandlerList.Size(); messageHandlerIndex++)
				messageHandlerList[messageHandlerIndex]->OnInternalPacket(internalPacket, sendPacketCount, playerId, (RakNetTime)(time/(RakNetTimeNS)1000), true);

			if (writeFalseToHeader)
			{
				output->Write(false);
				writeFalseToHeader=false;
			}
			statistics.messageTotalBitsSent[ i ] += WriteToBitStreamFromInternalPacket( output, internalPacket );
			//output->PrintBits();
			internalPacket->packetNumber=sendPacketCount;
			messagesSent++;

			if ( isReliable )
			{
				// Reliable packets are saved to resend later
				reliableBits += internalPacket->dataBitLength;
				internalPacket->nextActionTime = time + ackTimeIncrement;
				internalPacket->histogramMarker=histogramReceiveMarker;
				resendList.Insert( internalPacket->messageNumber, internalPacket);
				//printf("ackTimeIncrement=%i\n", ackTimeIncrement/1000);
				InsertPacketIntoResendList( internalPacket, time, false, true);
				*reliableDataSent = true;
			}
			else
			{
				// Unreliable packets are deleted
				if (internalPacket->data)
				delete [] internalPacket->data;
				internalPacketPool.ReleasePointer( internalPacket );
			}
		}
	}

END_OF_GENERATE_FRAME:
	;

	// if (output->GetNumberOfBitsUsed()>0)
	// {
	// Update the throttle with the header
	//  bytesSent+=output->GetNumberOfBytesUsed() + UDP_HEADER_SIZE;
	//}

	if (output->GetNumberOfBitsUsed()>0)
		sendPacketCount++;

	return messagesSent;
}

typedef char R5ReliabilityComplete[(sizeof(ReliabilityLayer)==1794)?1:-1];
typedef char R5PacketComplete[(sizeof(InternalPacket)==55)?1:-1];

typedef char R5ReliableCountersOffset[(offsetof(RakNetStatisticsStruct,_pad20)==0x20)?1:-1];
typedef char R5ReliableCounterWidth[(sizeof(unsigned)==4)?1:-1];

// Original same-TU ReliabilityLayer callees, copied verbatim from accepted owners.
int ReliabilityLayer::GetBitStreamHeaderLength( const InternalPacket *const internalPacket )
{
#ifdef _DEBUG
	assert( internalPacket );
#endif

	int bitLength;

	bitLength=sizeof(MessageNumberType)*2*8;

	// Write the PacketReliability.  This is encoded in 3 bits
	//bitStream->WriteBits((unsigned char*)&(internalPacket->reliability), 3, true);
	bitLength += reliabilitySizeInBits;

	// If the reliability requires an ordering channel and ordering index, we Write those.
	if ( internalPacket->reliability == UNRELIABLE_SEQUENCED || internalPacket->reliability == RELIABLE_SEQUENCED || internalPacket->reliability == RELIABLE_ORDERED )
	{
		// ordering channel encoded in 5 bits (from 0 to 31)
		//bitStream->WriteBits((unsigned char*)&(internalPacket->orderingChannel), 5, true);
		bitLength+=5;

		// ordering index is one byte
		//bitStream->WriteCompressed(internalPacket->orderingIndex);
		bitLength+=sizeof(OrderingIndexType)*8;
	}

	// Write if this is a split packet (1 bit)
	bool isSplitPacket = internalPacket->splitPacketCount > 0;

	//bitStream->Write(isSplitPacket);
	bitLength += 1;

	if ( isSplitPacket )
	{
		// split packet indices are two bytes (so one packet can be split up to 65535
		// times - maximum packet size would be about 500 * 65535)
		//bitStream->Write(internalPacket->splitPacketId);
		//bitStream->WriteCompressed(internalPacket->splitPacketIndex);
		//bitStream->WriteCompressed(internalPacket->splitPacketCount);
		bitLength += (sizeof(SplitPacketIdType) + sizeof(SplitPacketIndexType) * 2) * 8;
	}

	// Write how many bits the packet data is. Stored in an unsigned short and
	// read from 16 bits
	//bitStream->WriteBits((unsigned char*)&(internalPacket->dataBitLength), 16, true);

	// Read how many bits the packet data is.  Stored in 16 bits
	bitLength += 16;

	// Byte alignment
	//bitLength += 8 - ((bitLength -1) %8 + 1);

	return bitLength;
}

int ReliabilityLayer::WriteToBitStreamFromInternalPacket( RakNet::BitStream *bitStream, const InternalPacket *const internalPacket )
{
    typedef char R5ReliabilityBitsOffset[(offsetof(ReliabilityLayer,reliabilitySizeInBits)==0x403)?1:-1];
    typedef char R5InternalPoolOffset[(offsetof(ReliabilityLayer,internalPacketPool)==0x6F2)?1:-1];

#ifdef _DEBUG
	assert( bitStream && internalPacket );
#endif

	int start = bitStream->GetNumberOfBitsUsed();
	const unsigned char c = (unsigned char) internalPacket->reliability;

	// testing
	//if (internalPacket->reliability==UNRELIABLE)
	//  printf("Sending unreliable packet %i\n", internalPacket->messageNumber);
	//else if (internalPacket->reliability==RELIABLE_SEQUENCED || internalPacket->reliability==RELIABLE_ORDERED || internalPacket->reliability==RELIABLE)
	//	  printf("Sending reliable packet number %i\n", internalPacket->messageNumber);

	//bitStream->AlignWriteToByteBoundary();

	// Write the message number (2 bytes)
	bitStream->Write( internalPacket->messageNumber );

	// Acknowledgment packets have no more data than the messageNumber and whether it is anacknowledgment


#ifdef _DEBUG
	assert( internalPacket->dataBitLength > 0 );
#endif

	// Write the PacketReliability.  This is encoded in 3 bits
	bitStream->WriteBits( (const unsigned char *)&c, reliabilitySizeInBits, true );

	// If the reliability requires an ordering channel and ordering index, we Write those.
	if ( internalPacket->reliability == UNRELIABLE_SEQUENCED || internalPacket->reliability == RELIABLE_SEQUENCED || internalPacket->reliability == RELIABLE_ORDERED )
	{
		// ordering channel encoded in 5 bits (from 0 to 31)
		bitStream->WriteBits( ( unsigned char* ) & ( internalPacket->orderingChannel ), 5, true );

		// One or two bytes
		bitStream->Write( internalPacket->orderingIndex );
	}

	// Write if this is a split packet (1 bit)
	bool isSplitPacket = internalPacket->splitPacketCount > 0;

	bitStream->Write( isSplitPacket );

	if ( isSplitPacket )
	{
		bitStream->Write( internalPacket->splitPacketId );
		bitStream->WriteCompressed( internalPacket->splitPacketIndex );
		bitStream->WriteCompressed( internalPacket->splitPacketCount );
	}

	// Write how many bits the packet data is. Stored in 13 bits
#ifdef _DEBUG
	assert( BITS_TO_BYTES( internalPacket->dataBitLength ) < MAXIMUM_MTU_SIZE ); // I never send more than MTU_SIZE bytes

#endif

	unsigned short length = ( unsigned short ) internalPacket->dataBitLength; // Ignore the 2 high bytes for WriteBits

	bitStream->WriteCompressed( length );

	// Write the actual data.
	bitStream->WriteAlignedBytes( ( unsigned char* ) internalPacket->data, BITS_TO_BYTES( internalPacket->dataBitLength ) );

	//bitStream->WriteBits((unsigned char*)internalPacket->data, internalPacket->dataBitLength);

	return bitStream->GetNumberOfBitsUsed() - start;
}

void ReliabilityLayer::InsertPacketIntoResendList( InternalPacket *internalPacket, RakNetTimeNS time, bool makeCopyOfInternalPacket, bool firstResend )
{
	// lastAckTime is the time we last got an acknowledgment - however we also initialize the value if this is the first resend and
	// either we never got an ack before or we are inserting into an empty resend queue
	if ( firstResend && (lastAckTime == 0 || resendList.IsEmpty()))
	{
		lastAckTime = time; // Start the timer for the ack of this packet if we aren't already waiting for an ack
	}

	if (makeCopyOfInternalPacket)
	{
		InternalPacket *pool=internalPacketPool.GetPointer();
		//printf("Adding %i\n", internalPacket->data);
		memcpy(pool, internalPacket, sizeof(InternalPacket));
		resendQueue.Push( pool );
	}
	else
	{
		RakAssert(internalPacket->nextActionTime!=0);

		resendQueue.Push( internalPacket );
	}

}
