#include "../raknet/ConnectionGraph.h"

// Preserve original semantics, including groupId self-assignment in the parameterized constructor.
ConnectionGraph::PlayerIdAndGroupId::PlayerIdAndGroupId()
{

}
ConnectionGraph::PlayerIdAndGroupId::~PlayerIdAndGroupId()
{

}
ConnectionGraph::PlayerIdAndGroupId::PlayerIdAndGroupId(PlayerID playerId, ConnectionGraphGroupID groupID)
{
	this->playerId=playerId;
	this->groupId=groupId;
}
bool ConnectionGraph::PlayerIdAndGroupId::operator==( const ConnectionGraph::PlayerIdAndGroupId& right ) const
{
	return playerId==right.playerId;
}
bool ConnectionGraph::PlayerIdAndGroupId::operator!=( const ConnectionGraph::PlayerIdAndGroupId& right ) const
{
	return playerId!=right.playerId;
}
bool ConnectionGraph::PlayerIdAndGroupId::operator > ( const ConnectionGraph::PlayerIdAndGroupId& right ) const
{
	return playerId>right.playerId;
}
bool ConnectionGraph::PlayerIdAndGroupId::operator < ( const ConnectionGraph::PlayerIdAndGroupId& right ) const
{
	return playerId<right.playerId;
}

typedef char R5NodeSize[(sizeof(ConnectionGraph::PlayerIdAndGroupId)==7)?1:-1];

// Real lower providers originally emitted with Node ctor/dtor definitions visible.
template class DataStructures::Map<ConnectionGraph::PlayerIdAndGroupId, unsigned short>;
template class DataStructures::Map<ConnectionGraph::PlayerIdAndGroupId, DataStructures::Map<ConnectionGraph::PlayerIdAndGroupId, unsigned short> *>;
template class DataStructures::Queue<ConnectionGraph::PlayerIdAndGroupId>;
template DataStructures::WeightedGraph<ConnectionGraph::PlayerIdAndGroupId, unsigned short, false>::WeightedGraph();
template DataStructures::WeightedGraph<ConnectionGraph::PlayerIdAndGroupId, unsigned short, false>::~WeightedGraph();
template void DataStructures::WeightedGraph<ConnectionGraph::PlayerIdAndGroupId, unsigned short, false>::AddNode(const ConnectionGraph::PlayerIdAndGroupId &);
template void DataStructures::WeightedGraph<ConnectionGraph::PlayerIdAndGroupId, unsigned short, false>::AddConnection(const ConnectionGraph::PlayerIdAndGroupId &, const ConnectionGraph::PlayerIdAndGroupId &, unsigned short);
template void DataStructures::WeightedGraph<ConnectionGraph::PlayerIdAndGroupId, unsigned short, false>::Clear();
template int DataStructures::defaultOrderedListComparison<ConnectionGraph::PlayerIdAndGroupId, ConnectionGraph::PlayerIdAndGroupId>(const ConnectionGraph::PlayerIdAndGroupId &, const ConnectionGraph::PlayerIdAndGroupId &);
