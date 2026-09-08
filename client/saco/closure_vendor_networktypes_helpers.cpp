// Isolated source TU for trial30e.  The NetworkID::peerToPeerMode
// definition is intentionally supplied by the existing active owner.
#include "../raknet/NetworkTypes.h"

#include <string.h>
#include <stdio.h>

#ifdef _COMPATIBILITY_1
#include "../raknet/Compatibility1Includes.h"
#elif defined(_WIN32)
#include <winsock2.h>
#include <stdlib.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

char* my_itoa(int value, char* result, int base)
{
	if (base < 2 || base > 16) { *result = 0; return result; }
	char* out = result;
	int quotient = value;
	int absQModB;

	do {
		absQModB = quotient % base;
		if (absQModB < 0)
			absQModB = -absQModB;
		*out = "0123456789abcdef"[absQModB];
		++out;
		quotient /= base;
	} while (quotient);

	if (value < 0 && base == 10) *out++ = '-';
	*out = 0;

	char* start = result;
	char temp;
	out--;
	while (start < out)
	{
		temp = *start;
		*start = *out;
		*out = temp;
		start++;
		out--;
	}

	return result;
}

void PlayerID::SetBinaryAddress(const char* str)
{
#ifdef _COMPATIBILITY_1
	binaryAddress = UNASSIGNED_PLAYER_ID.binaryAddress;
#else
	binaryAddress = inet_addr(str);
#endif
}

void NetworkID::SetPeerToPeerMode(bool isPeerToPeer)
{
	// peerToPeerMode is the pre-existing active data owner; do not define it here.
	peerToPeerMode = isPeerToPeer;
}
