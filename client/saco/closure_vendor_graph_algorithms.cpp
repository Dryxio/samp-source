// Explicit instantiation of the real source graph algorithm used by Router.
// No Router/plugin callbacks, network execution, or fabricated wrapper.
#include "../raknet/ConnectionGraph.h"
#include "../raknet/DS_WeightedGraph.h"
#include "../raknet/DS_Heap.h"

// Suppress lower-provider emission in this Node-declaration-only context.
// Their sole definitions are instantiated in the real Node-definition context.
// extern template is the compiler's explicit-instantiation suppression mechanism.
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

typedef ConnectionGraph::PlayerIdAndGroupId R5GraphNode;
typedef DataStructures::WeightedGraph<R5GraphNode, unsigned short, false> R5WeightedGraph;
typedef DataStructures::Map<R5GraphNode, unsigned short> R5GraphAdjacency;
typedef DataStructures::Tree<R5GraphNode> R5GraphTree;
typedef DataStructures::Heap<unsigned short, R5GraphNode, false> R5GraphHeap;
typedef char R5GraphNodeComplete[(sizeof(R5GraphNode)==7)?1:-1];
typedef char R5WeightedGraphComplete[(sizeof(R5WeightedGraph)==52)?1:-1];
typedef char R5GraphAdjacencyComplete[(sizeof(R5GraphAdjacency)==24)?1:-1];
typedef char R5GraphTreeComplete[(sizeof(R5GraphTree)==19)?1:-1];
typedef char R5GraphHeapComplete[(sizeof(R5GraphHeap)==12)?1:-1];

// Emit the actual comparator definition, avoiding an unused compiler fallback name.
template int DataStructures::defaultOrderedListComparison<ConnectionGraph::PlayerIdAndGroupId, ConnectionGraph::PlayerIdAndGroupId>(const ConnectionGraph::PlayerIdAndGroupId &, const ConnectionGraph::PlayerIdAndGroupId &);
