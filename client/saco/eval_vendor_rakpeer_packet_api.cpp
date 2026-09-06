// Actual RakPeer queued-packet/RPC methods, original source order retained.
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <malloc.h>
#include "../raknet/RakNetDefines.h"
#include "../raknet/RakPeer.h"
#include "../raknet/GetTime.h"
#include "../raknet/PacketEnumerations.h"
#include "../raknet/PluginInterface.h"
#include "../raknet/RouterInterface.h"
#include "../raknet/RakAssert.h"

bool RakPeer::Send( const char *data, const int length, PacketPriority priority, PacketReliability reliability, char orderingChannel, PlayerID playerId, bool broadcast )
{
#ifdef _DEBUG
	assert( data && length > 0 );
#endif

	if ( data == 0 || length < 0 )
		return false;

	if ( remoteSystemList == 0 || endThreads == true )
		return false;

	if ( broadcast == false && playerId == UNASSIGNED_PLAYER_ID )
		return false;

	if (broadcast==false && router && GetIndexFromPlayerID(playerId)==-1)
	{
		return router->Send(data, BYTES_TO_BITS(length), priority, reliability, orderingChannel, playerId);
	}
	else
	{
		SendBuffered(data, length*8, priority, reliability, orderingChannel, playerId, broadcast, RemoteSystemStruct::NO_ACTION);
	}

	return true;
}

bool RakPeer::Send( RakNet::BitStream * bitStream, PacketPriority priority, PacketReliability reliability, char orderingChannel, PlayerID playerId, bool broadcast )
{
#ifdef _DEBUG
	assert( bitStream->GetNumberOfBytesUsed() > 0 );
#endif

	if ( bitStream->GetNumberOfBytesUsed() == 0 )
		return false;

	if ( remoteSystemList == 0 || endThreads == true )
		return false;

	if ( broadcast == false && playerId == UNASSIGNED_PLAYER_ID )
		return false;

	if (broadcast==false && router && GetIndexFromPlayerID(playerId)==-1)
	{
		return router->Send((const char*)bitStream->GetData(), bitStream->GetNumberOfBitsUsed(), priority, reliability, orderingChannel, playerId);
	}
	else
	{
		// Sends need to be buffered and processed in the update thread because the playerID associated with the reliability layer can change,
		// from that thread, resulting in a send to the wrong player!  While I could mutex the playerID, that is much slower than doing this
		SendBuffered((const char*)bitStream->GetData(), bitStream->GetNumberOfBitsUsed(), priority, reliability, orderingChannel, playerId, broadcast, RemoteSystemStruct::NO_ACTION);
	}
	
	return true;
}

Packet* RakPeer::Receive( void )
{
	Packet *packet = ReceiveIgnoreRPC();
	while (packet && (packet->data[ 0 ] == ID_RPC || (packet->length>sizeof(unsigned char)+sizeof(RakNetTime) && packet->data[0]==ID_TIMESTAMP && packet->data[sizeof(unsigned char)+sizeof(RakNetTime)]==ID_RPC)))
	{
		if (packet->playerId != UNASSIGNED_PLAYER_ID)
		{
			// Do RPC calls from the user thread, not the network update thread
			// If we are currently blocking on an RPC reply, send ID_RPC to the blocker to handle rather than handling RPCs automatically
			HandleRPCPacket( ( char* ) packet->data, packet->length, packet->playerId );
		}

		DeallocatePacket( packet );

		packet = ReceiveIgnoreRPC();
	}

    return packet;
}

Packet* RakPeer::ReceiveIgnoreRPC( void )
{
	if ( !( IsActive() ) )
		return 0;

	Packet *packet;
	Packet **threadPacket;
	PluginReceiveResult pluginResult;

	int offset;
	unsigned int i;

	for (i=0; i < messageHandlerList.Size(); i++)
	{
		messageHandlerList[i]->Update(this);
	}

	do 
	{
#ifdef _RAKNET_THREADSAFE
		rakPeerMutexes[transferToPacketQueue_Mutex].Lock();
#endif
		// Take all the messages off the queue so if the user pushes them back they are really pushed back, and not just at the end of the immediate write
		threadPacket=packetSingleProducerConsumer.ReadLock();
		while (threadPacket)
		{
			packet=*threadPacket;
			packetSingleProducerConsumer.ReadUnlock();
			threadPacket=packetSingleProducerConsumer.ReadLock();
			packetPool.Push(packet);
		}
#ifdef _RAKNET_THREADSAFE
		rakPeerMutexes[transferToPacketQueue_Mutex].Unlock();
#endif

#ifdef _MSC_VER
#pragma warning( disable : 4127 ) // warning C4127: conditional expression is constant
#endif

#ifdef _RAKNET_THREADSAFE
		rakPeerMutexes[packetPool_Mutex].Lock();
#endif
		if (packetPool.Size()==0)
		{
#ifdef _RAKNET_THREADSAFE
			rakPeerMutexes[packetPool_Mutex].Unlock();
#endif
			return 0;
		}

		packet = packetPool.Pop();
#ifdef _RAKNET_THREADSAFE
		rakPeerMutexes[packetPool_Mutex].Unlock();
#endif
		if ( ( packet->length >= sizeof(unsigned char) + sizeof( RakNetTime ) ) &&
			( (unsigned char) packet->data[ 0 ] == ID_TIMESTAMP ) )
		{
			offset = sizeof(unsigned char);
			ShiftIncomingTimestamp( packet->data + offset, packet->playerId );
		}
		if ( (unsigned char) packet->data[ 0 ] == ID_RPC_REPLY )
		{
			HandleRPCReplyPacket( ( char* ) packet->data, packet->length, packet->playerId );
			DeallocatePacket( packet );
			packet=0; // Will do the loop again and get another packet
		}
		else
		{
			for (i=0; i < messageHandlerList.Size(); i++)
			{
				pluginResult=messageHandlerList[i]->OnReceive(this, packet);
				if (pluginResult==RR_STOP_PROCESSING_AND_DEALLOCATE)
				{
					DeallocatePacket( packet );
					packet=0; // Will do the loop again and get another packet
					break; // break out of the enclosing for
				}
				else if (pluginResult==RR_STOP_PROCESSING)
				{
					packet=0;
					break;
				}
			}
		}

	} while(packet==0);

#ifdef _DEBUG
	assert( packet->data );
#endif

	return packet;
}

void RakPeer::DeallocatePacket( Packet *packet )
{
	if ( packet == 0 )
		return;

	if (packet->deleteData)
		if (packet->data)
			delete packet->data;

	free(packet);
}

void RakPeer::RegisterAsRemoteProcedureCall( char* uniqueID, void ( *functionPointer ) ( RPCParameters *rpcParms ) )
{
	if ( uniqueID == 0 || uniqueID[ 0 ] == 0 || functionPointer == 0 )
		return;

	rpcMap.AddIdentifierWithFunction(uniqueID[0], (void*)functionPointer, false);

	/*
	char uppercaseUniqueID[ 256 ];

	int counter = 0;

	while ( uniqueID[ counter ] )
	{
		uppercaseUniqueID[ counter ] = ( char ) toupper( uniqueID[ counter ] );
		counter++;
	}

	uppercaseUniqueID[ counter ] = 0;

	// Each id must be unique
//#ifdef _DEBUG
//	assert( rpcTree.IsIn( RPCNode( uppercaseUniqueID, functionName ) ) == false );
//#endif

	if (rpcTree.IsIn( RPCNode( uppercaseUniqueID, functionName ) ))
		return;

	rpcTree.Add( RPCNode( uppercaseUniqueID, functionName ) );
	*/
}

void RakPeer::RegisterClassMemberRPC( char* uniqueID, void *functionPointer )
{
	if ( uniqueID == 0 || uniqueID[ 0 ] == 0 || functionPointer == 0 )
		return;

	rpcMap.AddIdentifierWithFunction(uniqueID[0], functionPointer, true);
}

void RakPeer::PushBackPacket( Packet *packet, bool pushAtHead)
{
#ifdef _RAKNET_THREADSAFE
	rakPeerMutexes[packetPool_Mutex].Lock();
#endif
	RakAssert(packet);
	if (pushAtHead)
		packetPool.PushAtHead(packet);
	else
		packetPool.Push(packet);
#ifdef _RAKNET_THREADSAFE
	rakPeerMutexes[packetPool_Mutex].Unlock();
#endif
}

bool RakPeer::HandleRPCPacket( const char *data, int length, PlayerID playerId )
{
	// RPC BitStream format is
	// ID_RPC - unsigned char
	// The unique ID  - unsigned char
	// Number of bits of the data (int)
	// The data

	RakNet::BitStream incomingBitStream( (unsigned char *) data, length, false );
	unsigned char uniqueIdentifier;
	unsigned char *userData;
	RPCIndex rpcIndex;
	RPCNode *node;
	RPCParameters rpcParms;

	rpcParms.recipient=this;
	rpcParms.sender=playerId;

	// Note to self - if I change this format then I have to change the PacketLogger class too
	incomingBitStream.IgnoreBits(8);
	if (data[0]==ID_TIMESTAMP)
		incomingBitStream.IgnoreBits(8*(sizeof(RakNetTime)+sizeof(unsigned char)));

	incomingBitStream.Read(uniqueIdentifier);

	rpcIndex = rpcMap.GetIndexFromFunctionName(uniqueIdentifier);

	if ( incomingBitStream.ReadCompressed( rpcParms.numberOfBitsOfData ) == false )
	{
#ifdef _DEBUG
		assert( 0 ); // bitstream was not long enough.  Some kind of internal error
#endif
		return false;
	}

	if (rpcIndex==UNDEFINED_RPC_INDEX)
	{
		// Unregistered function
		//RakAssert(0);
		return false;
	}

	node = rpcMap.GetNodeFromIndex(rpcIndex);
	if (node==0)
	{
#ifdef _DEBUG
		assert( 0 ); // Should never happen except perhaps from threading errors?  No harm in checking anyway
#endif
		return false;
	}

	// Call the function
	if ( rpcParms.numberOfBitsOfData == 0 )
	{
		rpcParms.input=0;
		node->staticFunctionPointer( &rpcParms );
	}
	else
	{
		if ( incomingBitStream.GetNumberOfUnreadBits() == 0 )
		{
#ifdef _DEBUG
			assert( 0 );
#endif
			return false; // No data was appended!
		}

		// We have to copy into a new data chunk because the user data might not be byte aligned.
		bool usedAlloca=false;
#if !defined(_COMPATIBILITY_1)
		if (BITS_TO_BYTES( incomingBitStream.GetNumberOfUnreadBits() ) < MAX_ALLOCA_STACK_ALLOCATION)
		{
			userData = ( unsigned char* ) alloca( BITS_TO_BYTES( incomingBitStream.GetNumberOfUnreadBits() ) );
			usedAlloca=true;
		}
		else
#endif
			userData = new unsigned char[BITS_TO_BYTES(incomingBitStream.GetNumberOfUnreadBits())];


		// The false means read out the internal representation of the bitstream data rather than
		// aligning it as we normally would with user data.  This is so the end user can cast the data received
		// into a bitstream for reading
		if ( incomingBitStream.ReadBits( ( unsigned char* ) userData, rpcParms.numberOfBitsOfData, false ) == false )
		{
#ifdef _DEBUG
			assert( 0 );
#endif
			#if defined(_COMPATIBILITY_1)
			delete [] userData;
			#endif

			return false; // Not enough data to read
		}

		// Call the function callback
		rpcParms.input=userData;
		node->staticFunctionPointer( &rpcParms );

		if (usedAlloca==false)
			if (userData!=NULL)
				delete [] userData;
	}

	return true;
}

void RakPeer::HandleRPCReplyPacket( const char *data, int length, PlayerID playerId )
{
	if (blockOnRPCReply)
	{
		if ((playerId==replyFromTargetPlayer && replyFromTargetBroadcast==false) ||
			(playerId!=replyFromTargetPlayer && replyFromTargetBroadcast==true))
		{
			replyFromTargetBS->Write(data+1, length-1);
			blockOnRPCReply=false;
		}
	}
}

void RakPeer::SendBuffered( const char *data, int numberOfBitsToSend, PacketPriority priority, PacketReliability reliability, char orderingChannel, PlayerID playerId, bool broadcast, RemoteSystemStruct::ConnectMode connectionMode )
{
#ifdef _DEBUG
	assert(orderingChannel >=0 && orderingChannel < 32);
#endif


	BufferedCommandStruct *bcs;

#ifdef _RAKNET_THREADSAFE
	rakPeerMutexes[bufferedCommands_Mutex].Lock();
#endif
	bcs=bufferedCommands.WriteLock();
	bcs->data = new char[BITS_TO_BYTES(numberOfBitsToSend)]; // Making a copy doesn't lose efficiency because I tell the reliability layer to use this allocation for its own copy
#ifdef _DEBUG
	assert(bcs->data);
#endif
	memcpy(bcs->data, data, BITS_TO_BYTES(numberOfBitsToSend));
    bcs->numberOfBitsToSend=numberOfBitsToSend;
	bcs->priority=priority;
	bcs->reliability=reliability;
	bcs->orderingChannel=orderingChannel;
	bcs->playerId=playerId;
	bcs->broadcast=broadcast;
	bcs->connectionMode=connectionMode;
	bcs->command=BufferedCommandStruct::BCS_SEND;
	bufferedCommands.WriteUnlock();

#ifdef _RAKNET_THREADSAFE
	rakPeerMutexes[bufferedCommands_Mutex].Unlock();
#endif
}
