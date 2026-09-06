/// \file
///
/// This file is part of RakNet Copyright 2003 Kevin Jenkins.
///
/// Usage of RakNet is subject to the appropriate license agreement.
/// Creative Commons Licensees are subject to the
/// license found at
/// http://creativecommons.org/licenses/by-nc/2.5/
/// Single application licensees are subject to the license found at
/// http://www.rakkarsoft.com/SingleApplicationLicense.html
/// Custom license users are subject to the terms therein.
/// GPL license users are subject to the GNU General Public
/// License as published by the Free
/// Software Foundation; either version 2 of the License, or (at your
/// option) any later version.

#include "../raknet/StringCompressor.h"
#include "../raknet/DS_HuffmanEncodingTree.h"
#include "../raknet/BitStream.h"
#include <assert.h>
#include <string.h>
#include <memory.h>

StringCompressor* StringCompressor::instance=0;
int StringCompressor::referenceCount=0;

void StringCompressor::AddReference(void)
{
	if (++referenceCount==1)
	{
		instance = new StringCompressor;
	}
}
void StringCompressor::RemoveReference(void)
{
	assert(referenceCount > 0);

	if (referenceCount > 0)
	{
		if (--referenceCount==0)
		{
			delete instance;
			instance=0;
		}
	}
}

StringCompressor* StringCompressor::Instance(void)
{
	return instance;
}

extern unsigned int englishCharacterFrequencies[256];

StringCompressor::StringCompressor()
{
	DataStructures::Map<int, HuffmanEncodingTree *>::IMPLEMENT_DEFAULT_COMPARISON();

	// Make a default tree immediately, since this is used for RPC possibly from multiple threads at the same time
	HuffmanEncodingTree *huffmanEncodingTree = new HuffmanEncodingTree;
	huffmanEncodingTree->GenerateFromFrequencyTable( englishCharacterFrequencies );
	huffmanEncodingTrees.Set(0, huffmanEncodingTree);
}
void StringCompressor::GenerateTreeFromStrings( unsigned char *input, unsigned inputLength, int languageID )
{
	HuffmanEncodingTree *huffmanEncodingTree;
	if (huffmanEncodingTrees.Has(languageID))
	{
		huffmanEncodingTree = huffmanEncodingTrees.Get(languageID);
		delete huffmanEncodingTree;
	}

	unsigned index;
	unsigned int frequencyTable[ 256 ];

	if ( inputLength == 0 )
		return ;

	// Zero out the frequency table
	memset( frequencyTable, 0, sizeof( frequencyTable ) );

	// Generate the frequency table from the strings
	for ( index = 0; index < inputLength; index++ )
		frequencyTable[ input[ index ] ] ++;

	// Build the tree
	huffmanEncodingTree = new HuffmanEncodingTree;
	huffmanEncodingTree->GenerateFromFrequencyTable( frequencyTable );
	huffmanEncodingTrees.Set(languageID, huffmanEncodingTree);
}

StringCompressor::~StringCompressor()
{
	for (unsigned i=0; i < huffmanEncodingTrees.Size(); i++)
		delete huffmanEncodingTrees[i];
}

void StringCompressor::EncodeString( const char *input, int maxCharsToWrite, RakNet::BitStream *output, int languageID )
{
	HuffmanEncodingTree *huffmanEncodingTree;
	if (huffmanEncodingTrees.Has(languageID)==false)
		return;
	huffmanEncodingTree=huffmanEncodingTrees.Get(languageID);

	if ( input == 0 )
	{
		output->WriteCompressed( (unsigned short) 0 );
		return ;
	}

	RakNet::BitStream encodedBitStream;

	unsigned short stringBitLength;

	int charsToWrite;

	if ( maxCharsToWrite<=0 || ( int ) strlen( input ) < maxCharsToWrite )
		charsToWrite = ( int ) strlen( input );
	else
		charsToWrite = maxCharsToWrite - 1;

	huffmanEncodingTree->EncodeArray( ( unsigned char* ) input, charsToWrite, &encodedBitStream );

	stringBitLength = ( unsigned short ) encodedBitStream.GetNumberOfBitsUsed();

	output->WriteCompressed( stringBitLength );

	output->WriteBits( encodedBitStream.GetData(), stringBitLength );
}

bool StringCompressor::DecodeString( char *output, int maxCharsToWrite, RakNet::BitStream *input, int languageID )
{
	HuffmanEncodingTree *huffmanEncodingTree;
	if (huffmanEncodingTrees.Has(languageID)==false)
		return false;
	huffmanEncodingTree=huffmanEncodingTrees.Get(languageID);

	unsigned short stringBitLength;
	int bytesInStream;

	output[ 0 ] = 0;

	if ( input->ReadCompressed( stringBitLength ) == false )
		return false;

	if ( input->GetNumberOfUnreadBits() < stringBitLength )
		return false;

	bytesInStream = huffmanEncodingTree->DecodeArray( input, stringBitLength, maxCharsToWrite, ( unsigned char* ) output );

	if ( bytesInStream < maxCharsToWrite )
		output[ bytesInStream ] = 0;
	else
		output[ maxCharsToWrite - 1 ] = 0;

	return true;
}

typedef char R5StringCompressorSize[(sizeof(StringCompressor)==21)?1:-1];
typedef char R5StringHuffmanSize[(sizeof(HuffmanEncodingTree)==1540)?1:-1];
