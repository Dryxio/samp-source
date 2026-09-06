#include "../raknet/NetworkIDGenerator.h"

bool NetworkIDNode::operator==( const NetworkIDNode& right ) const
{
	if ( networkID == right.networkID )
		return !0;

	return 0;
}

bool NetworkIDNode::operator > ( const NetworkIDNode& right ) const
{
	if ( networkID > right.networkID )
		return !0;

	return 0;
}

bool NetworkIDNode::operator < ( const NetworkIDNode& right ) const
{
	if ( networkID < right.networkID )
		return !0;

	return 0;
}

NetworkIDNode::NetworkIDNode()
{
	object = 0;
}

NetworkIDNode::NetworkIDNode( NetworkID _networkID, NetworkIDGenerator *_object )
{
	networkID = _networkID;
	object = _object;
}



// Instantiate only the real tree family; no NetworkIDGenerator virtual caller.
template DataStructures::BinarySearchTree<NetworkIDNode>::BinarySearchTree();
template DataStructures::BinarySearchTree<NetworkIDNode>::~BinarySearchTree();
template DataStructures::AVLBalancedBinarySearchTree<NetworkIDNode>::AVLBalancedBinarySearchTree();
template DataStructures::AVLBalancedBinarySearchTree<NetworkIDNode>::~AVLBalancedBinarySearchTree();
template void DataStructures::AVLBalancedBinarySearchTree<NetworkIDNode>::Add(const NetworkIDNode &);
template void DataStructures::AVLBalancedBinarySearchTree<NetworkIDNode>::Del(const NetworkIDNode &);
template void DataStructures::BinarySearchTree<NetworkIDNode>::Clear();
template NetworkIDNode *& DataStructures::BinarySearchTree<NetworkIDNode>::GetPointerToNode(const NetworkIDNode &);
typedef char R5NetworkIDSize[(sizeof(NetworkID)==8)?1:-1];
typedef char R5NetworkIDNodeSize[(sizeof(NetworkIDNode)==12)?1:-1];
typedef char R5BinaryTreeSize[(sizeof(DataStructures::BinarySearchTree<NetworkIDNode>)==16)?1:-1];
typedef char R5AVLTreeSize[(sizeof(DataStructures::AVLBalancedBinarySearchTree<NetworkIDNode>)==16)?1:-1];
typedef char R5TreeLinkSize[(sizeof(DataStructures::BinarySearchTree<NetworkIDNode>::node)==12)?1:-1];
