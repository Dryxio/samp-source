// Complete vendor ACK insertion and resend deletion; actual template providers emitted by their owning headers.
#include "../raknet/ReliabilityLayer.h"
#include "../raknet/RakAssert.h"

template int DataStructures::RangeNodeComp<unsigned short>(const unsigned short &, const DataStructures::RangeNode<unsigned short> &);

// Explicit instantiation of the real wire-range methods in their original TU use order.
// These are complete existing templates, not forwarding wrappers.
template bool DataStructures::RangeList<unsigned short>::Deserialize(RakNet::BitStream *);
template unsigned DataStructures::RangeList<unsigned short>::Serialize(RakNet::BitStream *, int, bool);

unsigned ReliabilityLayer::RemovePacketFromResendListAndDeleteOlderReliableSequenced( const MessageNumberType messageNumber, RakNetTimeNS time )
{
	InternalPacket * internalPacket;
	//InternalPacket *temp;
	PacketReliability reliability; // What type of reliability algorithm to use with this packet
	unsigned char orderingChannel; // What ordering channel this packet is on, if the reliability type uses ordering channels
	OrderingIndexType orderingIndex; // The ID used as identification for ordering channels
//	unsigned j;

	bool deleted;
	deleted=resendList.Delete(messageNumber, internalPacket);
	if (deleted)
	{
		reliability = internalPacket->reliability;
		orderingChannel = internalPacket->orderingChannel;
		orderingIndex = internalPacket->orderingIndex;
//		delete [] internalPacket->data;
//		internalPacketPool.ReleasePointer( internalPacket );
		internalPacket->nextActionTime=0; // Will be freed in the update function
		return internalPacket->histogramMarker;

		// Rarely used and thus disabled for speed
		/*
		// If the deleted packet was reliable sequenced, also delete all older reliable sequenced resends on the same ordering channel.
		// This is because we no longer need to send these.
		if ( reliability == RELIABLE_SEQUENCED )
		{
			unsigned j = 0;
			while ( j < resendList.Size() )
			{
				internalPacket = resendList[ j ];

				if ( internalPacket && internalPacket->reliability == RELIABLE_SEQUENCED && internalPacket->orderingChannel == orderingChannel && IsOlderOrderedPacket( internalPacket->orderingIndex, orderingIndex ) )
				{
					// Delete the packet
					delete [] internalPacket->data;
					internalPacketPool.ReleasePointer( internalPacket );
					resendList[ j ] = 0; // Generate a hole
				}

				j++;
			}

		}
		*/
	}
	else
	{

		statistics.duplicateAcknowlegementsReceived++;
	}

	return (unsigned)-1;
}

void ReliabilityLayer::SendAcknowledgementPacket( const MessageNumberType messageNumber, RakNetTimeNS time )
{
	statistics.acknowlegementsSent++;
 	acknowlegements.Insert(messageNumber);
}

typedef char R5ReliabilityComplete[(sizeof(ReliabilityLayer)==1794)?1:-1];
typedef char R5InternalPacketComplete[(sizeof(InternalPacket)==55)?1:-1];
