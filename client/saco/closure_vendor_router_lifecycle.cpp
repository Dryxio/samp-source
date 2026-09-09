// Direct vendor Router lifecycle with the genuine multiple-inheritance layout.
#include "../raknet/Router.h"
Router::Router()
{
    graph=0;
    restrictByType=false;
    rakPeer=0;
    DataStructures::OrderedList<unsigned char,unsigned char>::IMPLEMENT_DEFAULT_COMPARISON();
}
Router::~Router() {}
void Router::OnDisconnect(RakPeerInterface *peer) {}
void Router::Update(RakPeerInterface *peer) {}
void Router::OnCloseConnection(RakPeerInterface *peer, PlayerID playerId) {}
typedef char R5RouterSize[(sizeof(Router)==29)?1:-1];
