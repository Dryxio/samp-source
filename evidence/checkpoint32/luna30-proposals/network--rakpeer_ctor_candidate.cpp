// UNCOMPILED research handoff.  Normal C++ reconstruction only.
//
// Provenance: vendor/upstream/raknet/RakPeer.cpp, RakPeer::RakPeer source
// lines 164-208.  The RemoteSystemStruct default constructor is the
// compiler-generated layout constructor emitted by the same original TU;
// explicitly defaulting it here preserves that source/layout provenance.
// This TU is intended to be staged as client/saco/rakpeer_ctor_candidate.cpp.

#include <string.h>

// The original RakPeer member layout is the contract.  protected is exposed
// only for explicit instantiation of the actual member-template types; no
// class layout or declaration is changed.
#define protected public
#include "../raknet/RakPeer.h"
#undef protected

#include "../raknet/StringCompressor.h"
#include "../raknet/StringTable.h"

RakPeer::RakPeer()
{
	StringCompressor::AddReference();
	StringTable::AddReference();

#if !defined(_COMPATIBILITY_1)
	usingSecurity = false;
#endif
	memset( frequencyTable, 0, sizeof( unsigned int ) * 256 );
	rawBytesSent = rawBytesReceived = compressedBytesSent = compressedBytesReceived = 0;
	outputTree = inputTree = 0;
	connectionSocket = INVALID_SOCKET;
	MTUSize = DEFAULT_MTU_SIZE;
	trackFrequencyTable = false;
	maximumIncomingConnections = 0;
	maximumNumberOfPeers = 0;
	remoteSystemList = 0;
	bytesSentPerSecond = bytesReceivedPerSecond = 0;
	endThreads = true;
	isMainLoopThreadActive = false;
	occasionalPing = false;
	connectionSocket = INVALID_SOCKET;
	myPlayerId = UNASSIGNED_PLAYER_ID;
	allowConnectionResponseIPMigration = false;
	blockOnRPCReply=false;
	incomingPasswordLength=0;
	router=0;
	splitMessageProgressInterval=0;
	unreliableTimeout=0;

#if defined (_WIN32) && defined(USE_WAIT_FOR_MULTIPLE_EVENTS)
	recvEvent = INVALID_HANDLE_VALUE;
#endif

#ifndef _RELEASE
	_maxSendBPS=0.0;
	_minExtraPing=0;
	_extraPingVariance=0;
#endif
}

RakPeer::RemoteSystemStruct::RemoteSystemStruct() = default;

// These are the exact compiler-generated providers called by the original
// RakPeer constructor and absent from the accepted 428758-byte union.  They
// use only the vendor template definitions and the unchanged RakPeer layout.
template DataStructures::List<RakPeer::BanStruct*>::List();
template DataStructures::List<PluginInterface*>::List();
template DataStructures::List<PlayerIDAndIndex>::List();
template DataStructures::OrderedList<PlayerID, PlayerIDAndIndex, PlayerIDAndIndexComp>::OrderedList();
