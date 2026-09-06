// Real Router receive/filter/tree serialization methods only; no graph construction or fake VFT.
#include <string.h>
#include <assert.h>
#include "../raknet/Router.h"
#include "../raknet/BitStream.h"
#include "../raknet/RakPeerInterface.h"
#include "../raknet/PacketEnumerations.h"
#include "../raknet/RakAssert.h"

// Genuine comparator instantiation required by the selected OrderedList<unsigned char> methods.
template int DataStructures::defaultOrderedListComparison<unsigned char,unsigned char>(const unsigned char &, const unsigned char &);

void Router::SetRestrictRoutingByType(bool restrict)
{
	restrictByType=restrict;
}

void Router::AddAllowedType(unsigned char messageId)
{
	if (allowedTypes.HasData(messageId)==false)
		allowedTypes.Insert(messageId,messageId);
}

void Router::RemoveAllowedType(unsigned char messageId)
{
	if (allowedTypes.HasData(messageId)==true)
		allowedTypes.Remove(messageId);
}

void Router::SetConnectionGraph(DataStructures::WeightedGraph<ConnectionGraph::PlayerIdAndGroupId, unsigned short, false> *connectionGraph)
{
	graph=connectionGraph;
}

void Router::SendTree(PacketPriority priority, PacketReliability reliability, char orderingChannel, DataStructures::Tree<ConnectionGraph::PlayerIdAndGroupId> *tree, const char *data, unsigned bitLength, RakNet::BitStream *out, SystemAddressList *recipients)
{
	unsigned outputOffset;

	// Write routing identifer
	out->Write((unsigned char)ID_ROUTE_AND_MULTICAST);

	// Write the send parameters
	out->WriteCompressed((unsigned char)priority);
	out->WriteCompressed((unsigned char)reliability);
	out->WriteCompressed((unsigned char)orderingChannel);

	// Write the user payload length
	out->Write((unsigned int)bitLength);
//	out->PrintBits();
//	payload->PrintBits();

	out->AlignWriteToByteBoundary();
//	payload->AlignReadToByteBoundary();
//	out->Write(payload, payload->GetNumberOfUnreadBits());
//	out->PrintBits();
	if ((bitLength % 8)==0)
		out->Write(data, BITS_TO_BYTES(bitLength));
	else
		out->WriteBits((const unsigned char*)data, bitLength, false);

	// Save where to start writing per-system data
	outputOffset=out->GetWriteOffset();

	// Write every child of the root of the tree (PlayerID, isRecipient, branch)
	unsigned i;
	for (i=0; i < tree->children.Size(); i++)
	{
		// Start writing at the per-system data byte
		out->SetWriteOffset(outputOffset);

		// Write our external IP to designate the sender
		out->Write(rakPeer->GetExternalID(tree->children[i]->data.playerId));

		// Serialize the tree
		SerializePreorder(tree->children[i], out, recipients);

		// Send to the first hop
#ifdef _DO_PRINTF
		printf("%i sending to %i\n", rakPeer->GetExternalID(tree->children[i]->data).port, tree->children[i]->data.port);
#endif
		rakPeer->Send(out, priority, reliability, orderingChannel, tree->children[i]->data.playerId, false);
	}
}

PluginReceiveResult Router::OnReceive(RakPeerInterface *peer, Packet *packet)
{
	if (packet->data[0]==ID_ROUTE_AND_MULTICAST ||
		(packet->length>5 && packet->data[0]==ID_TIMESTAMP && packet->data[5]==ID_ROUTE_AND_MULTICAST))
	{
#ifdef _DO_PRINTF
		printf("%i got routed message from %i\n", peer->GetExternalID(packet->playerId).port, packet->playerId.port);
#endif
		RakNetTime timestamp;
		PacketPriority priority;
		PacketReliability reliability;
		unsigned char orderingChannel;
		PlayerID originalSender;
		RakNet::BitStream out;
		unsigned outStartingOffset;
		unsigned int payloadBitLength;
		unsigned payloadWriteByteOffset;
		RakNet::BitStream incomingBitstream(packet->data, packet->length, false);
		incomingBitstream.IgnoreBits(8);
		
		if (packet->data[0]==ID_TIMESTAMP)
		{
			incomingBitstream.Read(timestamp);
			out.Write((unsigned char)ID_TIMESTAMP);
			out.Write(timestamp);
		}

		// Read the send parameters
		unsigned char c;
		incomingBitstream.ReadCompressed(c);
		priority=(PacketPriority)c;
		incomingBitstream.ReadCompressed(c);
		reliability=(PacketReliability)c;
		incomingBitstream.ReadCompressed(orderingChannel);
		incomingBitstream.Read(payloadBitLength);
		
		out.Write((unsigned char) ID_ROUTE_AND_MULTICAST);
		out.WriteCompressed((unsigned char)priority);
		out.WriteCompressed((unsigned char)reliability);
		out.WriteCompressed((unsigned char)orderingChannel);
		out.Write(payloadBitLength);
		out.AlignWriteToByteBoundary();
		incomingBitstream.AlignReadToByteBoundary();
		payloadWriteByteOffset=BITS_TO_BYTES(out.GetWriteOffset());
		out.Write(&incomingBitstream, payloadBitLength); // This write also does a read on incomingBitStream

		if (restrictByType)
		{
			RakNet::BitStream t(out.GetData()+payloadWriteByteOffset, sizeof(unsigned char), false);
			unsigned char messageID;
			t.Read(messageID);
			if (allowedTypes.HasData(messageID)==false)
				return RR_STOP_PROCESSING_AND_DEALLOCATE; // Don't route restricted types
		}

		incomingBitstream.Read(originalSender);
		out.Write(originalSender);
		outStartingOffset=out.GetWriteOffset();

		// Deserialize the root
		bool hasData;
		PlayerID recipient;
		unsigned short numberOfChildren;
		incomingBitstream.Read(hasData);
		incomingBitstream.Read(recipient); // This should be our own address
		if (incomingBitstream.ReadCompressed(numberOfChildren)==false)
		{
#ifdef _DEBUG
			assert(0);
#endif
			return RR_STOP_PROCESSING_AND_DEALLOCATE;
		}

		unsigned childIndex;
		bool childHasData;
		PlayerID childRecipient;
		unsigned short childNumberOfChildren;
		PlayerID immediateRecipient;
		immediateRecipient=UNASSIGNED_PLAYER_ID;
		int pendingNodeCount=0;

		for (childIndex=0; childIndex < numberOfChildren; childIndex++)
		{
			while (pendingNodeCount!=-1)
			{
				// Copy out the serialized subtree for this child
				incomingBitstream.Read(childHasData);
				incomingBitstream.Read(childRecipient);
				if (!incomingBitstream.ReadCompressed(childNumberOfChildren))
					return RR_STOP_PROCESSING_AND_DEALLOCATE;
				if (immediateRecipient==UNASSIGNED_PLAYER_ID)
				{
					immediateRecipient=childRecipient;
				}

				pendingNodeCount+=childNumberOfChildren-1;

				out.Write(childHasData);
				out.Write(childRecipient);
				out.WriteCompressed(childNumberOfChildren);
			}

#ifdef _DO_PRINTF
			printf("%i routing to %i\n", peer->GetExternalID(packet->playerId).port, immediateRecipient.port);
#endif

			// Send what we got so far
			rakPeer->Send(&out, priority, reliability, orderingChannel, immediateRecipient, false);

			// Restart writing the per recipient data
			out.SetWriteOffset(outStartingOffset);

			// Reread the top level node
			immediateRecipient=UNASSIGNED_PLAYER_ID;

			pendingNodeCount=0;

		}

		// Write the user payload to the packet struct if this is a destination and change the sender and return true
		if (hasData)
		{
#ifdef _DO_PRINTF
			printf("%i returning payload to user\n", peer->GetExternalID(packet->playerId).port);
#endif

			if (packet->data[0]==ID_TIMESTAMP )
			{
				memcpy( packet->data + sizeof(RakNetTime)+sizeof(unsigned char), out.GetData()+payloadWriteByteOffset, BITS_TO_BYTES(payloadBitLength) );
				packet->bitSize=sizeof(RakNetTime)+sizeof(unsigned char)+payloadBitLength;
			}
			else
			{
				memcpy( packet->data, out.GetData()+payloadWriteByteOffset, BITS_TO_BYTES(payloadBitLength) );
				packet->bitSize=payloadBitLength;
			}
			packet->length=BITS_TO_BYTES(packet->bitSize);
			packet->playerIndex=65535;
			packet->playerId=originalSender;

			return RR_CONTINUE_PROCESSING;
		}

		// Absorb
		return RR_STOP_PROCESSING_AND_DEALLOCATE;
	}

	return RR_CONTINUE_PROCESSING;
}

void Router::OnAttach(RakPeerInterface *peer)
{
    rakPeer=peer;
	peer->SetRouterInterface(this);
}

void Router::OnDetach(RakPeerInterface *peer)
{
	peer->RemoveRouterInterface(this);
}

void Router::SerializePreorder(DataStructures::Tree<ConnectionGraph::PlayerIdAndGroupId> *tree, RakNet::BitStream *out, SystemAddressList *recipients) const
{
	unsigned i;
	out->Write((bool) (recipients->GetList()->GetIndexOf(tree->data.playerId)!=MAX_UNSIGNED_LONG));
	out->Write(tree->data.playerId);
	out->WriteCompressed((unsigned short) tree->children.Size());
	for (i=0; i < tree->children.Size(); i++)
		SerializePreorder(tree->children[i], out, recipients);
}
