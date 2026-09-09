// Direct vendor Router Send overloads; real class and graph/list types.
#include <string.h>
#include "../raknet/Router.h"
#include "../raknet/BitStream.h"
#include "../raknet/RakPeerInterface.h"
#include "../raknet/PacketEnumerations.h"
#include "../raknet/RakAssert.h"

// Ownership hypothesis grounded in closure_vendor_graph_algorithms.cpp:
// preserve calls to the already accepted Node-definition-context providers.
// Emitting these templates here instead would create different COMDATs because
// ConnectionGraph::PlayerIdAndGroupId has only declarations in this TU.
extern template class DataStructures::Map<ConnectionGraph::PlayerIdAndGroupId, unsigned short>;
extern template class DataStructures::Map<ConnectionGraph::PlayerIdAndGroupId, DataStructures::Map<ConnectionGraph::PlayerIdAndGroupId, unsigned short> *>;
extern template class DataStructures::Queue<ConnectionGraph::PlayerIdAndGroupId>;
extern template class DataStructures::WeightedGraph<ConnectionGraph::PlayerIdAndGroupId, unsigned short, false>;

// Actual lower providers copy value types whose real destructors are empty.
// Their original Node-definition TU is non-throwing; declarations restore that
// information without emitting a competing provider in this algorithm TU.
template <> ConnectionGraph::PlayerIdAndGroupId DataStructures::Map<ConnectionGraph::PlayerIdAndGroupId, unsigned short>::GetKeyAtIndex(unsigned) const throw();
template <> ConnectionGraph::PlayerIdAndGroupId DataStructures::Map<ConnectionGraph::PlayerIdAndGroupId, DataStructures::Map<ConnectionGraph::PlayerIdAndGroupId, unsigned short> *>::GetKeyAtIndex(unsigned) const throw();
template <> ConnectionGraph::PlayerIdAndGroupId DataStructures::Queue<ConnectionGraph::PlayerIdAndGroupId>::Pop() throw();

template unsigned DataStructures::Map<ConnectionGraph::PlayerIdAndGroupId, DataStructures::Map<ConnectionGraph::PlayerIdAndGroupId, unsigned short> *>::Size(void) const;

template unsigned DataStructures::Queue<ConnectionGraph::PlayerIdAndGroupId>::Size(void) const;

template void DataStructures::WeightedGraph<ConnectionGraph::PlayerIdAndGroupId, unsigned short, false>::GenerateDisjktraMatrix(ConnectionGraph::PlayerIdAndGroupId, unsigned short);
template bool DataStructures::WeightedGraph<ConnectionGraph::PlayerIdAndGroupId, unsigned short, false>::GetShortestPath(DataStructures::List<ConnectionGraph::PlayerIdAndGroupId> &, ConnectionGraph::PlayerIdAndGroupId, ConnectionGraph::PlayerIdAndGroupId, unsigned short);

template bool DataStructures::WeightedGraph<ConnectionGraph::PlayerIdAndGroupId, unsigned short, false>::GetSpanningTree(
    DataStructures::Tree<ConnectionGraph::PlayerIdAndGroupId> &,
    DataStructures::List<ConnectionGraph::PlayerIdAndGroupId> *,
    ConnectionGraph::PlayerIdAndGroupId, unsigned short);


bool Router::Send( const char *data, unsigned bitLength, PacketPriority priority, PacketReliability reliability, char orderingChannel, PlayerID playerId )
{
	if (playerId!=UNASSIGNED_PLAYER_ID)
	{
		RakAssert(data);
		RakAssert(bitLength);
		// Prevent recursion in case a routing call itself calls the router
		if (bitLength>=8 && data[0]==ID_ROUTE_AND_MULTICAST)
			return false;

		SystemAddressList systemAddressList;
		systemAddressList.AddSystem(playerId);
		return Send((char*)data, bitLength, priority, reliability, orderingChannel, &systemAddressList);
	}
	return false;	
}
bool Router::Send( char *data, unsigned bitLength, PacketPriority priority, PacketReliability reliability, char orderingChannel, SystemAddressList *recipients )
{
	RakAssert(data);
	RakAssert(bitLength);
	if (recipients->GetList()->Size()==0)
		return false;
	if (bitLength==0)
		return false;
	DataStructures::Tree<ConnectionGraph::PlayerIdAndGroupId> tree;
	PlayerID root;
	root = rakPeer->GetExternalID(rakPeer->GetPlayerIDFromIndex(0));
	if (root==UNASSIGNED_PLAYER_ID)
		return false;
	DataStructures::List<ConnectionGraph::PlayerIdAndGroupId> recipientList;
	unsigned i;
	for (i=0; i < recipients->Size(); i++)
		recipientList.Insert(ConnectionGraph::PlayerIdAndGroupId(recipients->GetList()->operator [](i),0));
	if (graph->GetSpanningTree(tree, &recipientList, ConnectionGraph::PlayerIdAndGroupId(root,0), 65535)==false)
		return false;

	RakNet::BitStream out;

	// Write timestamp first, if the user had a timestamp
	if (data[0]==ID_TIMESTAMP && bitLength >= BYTES_TO_BITS(sizeof(MessageID)+sizeof(RakNetTime)))
	{
		out.Write(data, sizeof(MessageID)+sizeof(RakNetTime));
		data+=sizeof(MessageID)+sizeof(RakNetTime);
		bitLength-=BYTES_TO_BITS(sizeof(MessageID)+sizeof(RakNetTime));
	}

	SendTree(priority, reliability, orderingChannel, &tree, data, bitLength, &out, recipients);
	return true;
}

// Emit the actual comparator definition, matching its accepted algorithm owner.
template int DataStructures::defaultOrderedListComparison<ConnectionGraph::PlayerIdAndGroupId, ConnectionGraph::PlayerIdAndGroupId>(const ConnectionGraph::PlayerIdAndGroupId &, const ConnectionGraph::PlayerIdAndGroupId &);
