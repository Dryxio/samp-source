// UNCOMPILED salvage handoff; C++03/VS.NET2003-compatible normal C++ only.
// No RakPeer constructor, VFT, copied bytes, or symbolic assembly.
//
// The placement-new helper is an ordinary way to force the compiler to emit
// the true implicit RemoteSystemStruct default constructor. It does not define
// or imitate the constructor body. The four explicit instantiations are the
// actual vendor template providers used by RakPeer's member layout.

#include <new>

#define protected public
#include "../raknet/RakPeer.h"
#undef protected

// Keep this externally visible and out of line so the compiler must materialize
// the real implicit constructor call in the object. The helper is not a root.
__declspec(noinline) void R5ForceRemoteSystemStructCtor(void *storage)
{
	new (storage) RakPeer::RemoteSystemStruct();
}

template DataStructures::List<RakPeer::BanStruct*>::List();
template DataStructures::List<PluginInterface*>::List();
template DataStructures::List<PlayerIDAndIndex>::List();
template DataStructures::OrderedList<PlayerID, PlayerIDAndIndex, PlayerIDAndIndexComp>::OrderedList();
