// Original vendor SHA1 packet helpers and timestamp queue operations.
#include <string.h>
#include "../raknet/ReliabilityLayer.h"
void ReliabilityLayer::GetSHA1( unsigned char * const buffer, unsigned int
				nbytes, char code[ SHA1_LENGTH ] )
{
	CSHA1 sha1;

	sha1.Reset();
	sha1.Update( ( unsigned char* ) buffer, nbytes );
	sha1.Final();
	memcpy( code, sha1.GetHash(), SHA1_LENGTH );
}

//-------------------------------------------------------------------------------------------------------
// Check the SHA1 code
//-------------------------------------------------------------------------------------------------------
bool ReliabilityLayer::CheckSHA1( char code[ SHA1_LENGTH ], unsigned char *
				  const buffer, unsigned int nbytes )
{
	char code2[ SHA1_LENGTH ];
	GetSHA1( buffer, nbytes, code2 );

	for ( int i = 0; i < SHA1_LENGTH; i++ )
		if ( code[ i ] != code2[ i ] )
			return false;

	return true;
}


template void DataStructures::Queue<__int64>::Push(const __int64&);
template void DataStructures::Queue<__int64>::Compress();
