// Complete source. If accepted, replaces network_id_tree sole ownership.
#include "../raknet/NetworkIDGenerator.cpp"

// Retain accepted explicit tree instantiations in their real Node-definition context.
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

typedef char R5NetworkIDGeneratorSize[(sizeof(NetworkIDGenerator)==17)?1:-1];
