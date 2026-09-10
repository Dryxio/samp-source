#include "../raknet/ReliabilityLayer.h"
#include "../raknet/SocketLayer.h"
#include "../raknet/PluginInterface.h"
#include "../raknet/Rand.h"
#include "../raknet/RakAssert.h"
static const float PACKETLOSS_TOLERANCE=.02f;
static const double MINIMUM_SEND_BPS=144400.0;
static const RakNetTimeNS TIME_TO_NEW_SAMPLE=500000;
static const RakNetTimeNS MAX_TIME_TO_SAMPLE=250000;
void ReliabilityLayer::Update( SOCKET s, PlayerID playerId, int MTUSize, RakNetTimeNS time, DataStructures::List<PluginInterface*> &messageHandlerList )
{
#ifdef __USE_IO_COMPLETION_PORTS

	if ( readWriteSocket == INVALID_SOCKET )
		return;

	if (deadConnection)
		return;
#endif

	// This line is necessary because the timer isn't accurate
	if (time <= lastUpdateTime)
	{
		// Always set the last time in case of overflow
		lastUpdateTime=time;
		return;
	}

	RakNetTimeNS elapsedTime = time - lastUpdateTime;
	availableBandwidth+=currentBandwidth * ((double)elapsedTime/1000000.0f);
	if (availableBandwidth > currentBandwidth)
		availableBandwidth = currentBandwidth;
	lastUpdateTime=time;

	// unsigned resendListSize;
	bool reliableDataSent;
	UpdateThreadedMemory();

	// Due to thread vagarities and the way I store the time to avoid slow calls to RakNet::GetTime
	// time may be less than lastAck
	if ( resendList.IsEmpty()==false && time > lastAckTime && lastAckTime && time - lastAckTime > (RakNetTimeNS)timeoutTime*1000 )
	{
		// SHOW - dead connection
		// printf("The connection has been lost.\n");
		// We've waited a very long time for a reliable packet to get an ack and it never has
		deadConnection = true;
		return;
	}

	// Water canister has to have enough room to put more water in :)
	double requiredBuffer=(float)((MTUSize+UDP_HEADER_SIZE)*8);
	if (requiredBuffer > currentBandwidth)
		requiredBuffer=currentBandwidth;

	while ( availableBandwidth > requiredBuffer )
	{
		updateBitStream.Reset();
		GenerateDatagram( &updateBitStream, MTUSize, &reliableDataSent, time, playerId, messageHandlerList );
		if ( updateBitStream.GetNumberOfBitsUsed() > 0 )
		{
#ifndef _RELEASE
			if (minExtraPing > 0 || extraPingVariance > 0)
			{
				// Delay the send to simulate lag
				DataAndTime *dt;
				dt = new DataAndTime;
				memcpy( dt->data, updateBitStream.GetData(), updateBitStream.GetNumberOfBytesUsed() );
				dt->length = updateBitStream.GetNumberOfBytesUsed();
				dt->sendTime = time + (RakNetTimeNS)minExtraPing*1000;
				if (extraPingVariance > 0)
					dt->sendTime += ( randomMT() % (int)extraPingVariance );
				delayList.Insert( dt );
			}
			else
#endif
			SendBitStream( s, playerId, &updateBitStream );

			availableBandwidth-=updateBitStream.GetNumberOfBitsUsed()+UDP_HEADER_SIZE*8;
		}
		else
			break;
	}

	bool lastContinuousSend=continuousSend;
	continuousSend=availableBandwidth < requiredBuffer;

	if (continuousSend==true && lastContinuousSend==false)
	{
		histogramAckCount=0;
		histogramPlossCount=0;
		histogramStartTime=time+ping*2*1000;
		histogramEndTime=histogramStartTime+MAX_TIME_TO_SAMPLE;
		if (++histogramReceiveMarker==(unsigned)-1)
			histogramReceiveMarker=0;
	}

	if (time >= histogramEndTime )
	{
		float packetloss;

		double delta;
		if (histogramAckCount+histogramPlossCount)
			packetloss=(float)histogramPlossCount / ((float)histogramAckCount+(float)histogramPlossCount);
		else
			packetloss=0.0f; // This line can be true if we are sending only acks

		if (continuousSend==false)
		{
			if (packetloss > PACKETLOSS_TOLERANCE)
			{
				highBandwidth=currentBandwidth;
				if (packetloss > .2)
				{
					lowBandwidth/=2;
				}
				else
				{
					lowBandwidth*=.9;
				}

				if (lowBandwidth < MINIMUM_SEND_BPS)
					lowBandwidth=MINIMUM_SEND_BPS;

				delta = (highBandwidth-lowBandwidth)/2;
				currentBandwidth=delta+lowBandwidth;
				noPacketlossIncreaseCount=0;
			}
		}
		else
		{
			if (packetloss <= PACKETLOSS_TOLERANCE)
				lowBandwidth=currentBandwidth;
			else
				highBandwidth=currentBandwidth;

			if (packetloss==0.0)
			{
				// If no packetloss for many increases in a row, drop the high range and go into search mode.
				if (++noPacketlossIncreaseCount==10)
				{
					noPacketlossIncreaseCount=0;
					highBandwidth=0;
				}
			}
			else
				noPacketlossIncreaseCount=0;
			if (highBandwidth!=0.0)
			{
				// If a lot of packetloss at any time, decrease the low range by half
				if (packetloss > .2)
				{
					lowBandwidth/=2;
					if (lowBandwidth < MINIMUM_SEND_BPS)
						lowBandwidth=MINIMUM_SEND_BPS;
				}

				delta = (highBandwidth-lowBandwidth)/2;
				if (delta < MINIMUM_SEND_BPS/4)
				{
					// If no packetloss and done searching, increase the high range by 50%
					if (packetloss==0.0)
					{
						highBandwidth*=1.5;
					}
					else if (packetloss < PACKETLOSS_TOLERANCE)
					{
						// If some packetloss and done searching, increase the high range by 5%
						highBandwidth*=1.05;
					}
					else if (packetloss < PACKETLOSS_TOLERANCE*2)
					{
						// If some packetloss, but not a huge amount and done searching, decrease the low range by 10%
						lowBandwidth*=.9;
						if (lowBandwidth < MINIMUM_SEND_BPS)
							lowBandwidth=MINIMUM_SEND_BPS;
					}
					delta = (highBandwidth-lowBandwidth)/2;
				}
				currentBandwidth=delta+lowBandwidth;
			}
			else
			{
				// Don't know the maximum bandwidth, so keep doubling to find out
				currentBandwidth*=2.0;
			}
		}

		histogramPlossCount=0;
		histogramAckCount=0;
		histogramStartTime=time+TIME_TO_NEW_SAMPLE+ping*2*1000;
		histogramEndTime=histogramStartTime+MAX_TIME_TO_SAMPLE;
		if (++histogramReceiveMarker==(unsigned)-1)
			histogramReceiveMarker=0;
	}

#ifndef _RELEASE
	// Do any lagged sends
	unsigned i = 0;
	while ( i < delayList.Size() )
	{
		if ( delayList[ i ]->sendTime < time )
		{
			updateBitStream.Reset();
			updateBitStream.Write( delayList[ i ]->data, delayList[ i ]->length );
			// Send it now
			SendBitStream( s, playerId, &updateBitStream );

			delete delayList[ i ];
			if (i != delayList.Size() - 1)
				delayList[ i ] = delayList[ delayList.Size() - 1 ];
			delayList.Del();
		}

		else
			i++;
	}
#endif

}
void ReliabilityLayer::SendBitStream( SOCKET s, PlayerID playerId, RakNet::BitStream *bitStream )
{
	// SHOW - showing reliable flow
	// if (bitStream->GetNumberOfBytesUsed()>50)
	//  printf("Sending %i bytes. sendQueue[0].Size()=%i, resendList.Size()=%i\n", bitStream->GetNumberOfBytesUsed(), sendQueue[0].Size(),resendList.Size());

	int oldLength, length;

	// sentFrames++;

#ifndef _RELEASE
	if (maxSendBPS>0)
	{
		double chanceToLosePacket = (double)currentBandwidth / (double)maxSendBPS;
		if (frandomMT() < (float)chanceToLosePacket)
			return;

	}
#endif

	// Encode the whole bitstream if the encoder is defined.

	if ( encryptor.IsKeySet() )
	{
		length = bitStream->GetNumberOfBytesUsed();
		oldLength = length;

		encryptor.Encrypt( ( unsigned char* ) bitStream->GetData(), length, ( unsigned char* ) bitStream->GetData(), &length );
		statistics.encryptionBitsSent += ( length - oldLength ) * 8;

		assert( ( length % 16 ) == 0 );
	}
	else
	{
		length = bitStream->GetNumberOfBytesUsed();
	}

#ifdef __USE_IO_COMPLETION_PORTS
	if ( readWriteSocket == INVALID_SOCKET )
	{
		assert( 0 );
		return ;
	}

	statistics.packetsSent++;
	statistics.totalBitsSent += length * 8;
	SocketLayer::Instance()->Write( readWriteSocket, ( const char* ) bitStream->GetData(), length );
#else

	statistics.packetsSent++;
	statistics.totalBitsSent += length * 8;
	//printf("total bits=%i length=%i\n", BITS_TO_BYTES(statistics.totalBitsSent), length);

	SocketLayer::Instance()->SendTo( s, ( char* ) bitStream->GetData(), length, playerId.binaryAddress, playerId.port );
#endif // __USE_IO_COMPLETION_PORTS

	// lastPacketSendTime=time;
}
