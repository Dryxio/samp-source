#include "../raknet/NetworkTypes.h"
#include <winsock2.h>
#include <string.h>
#include <stdlib.h>
char *PlayerID::ToString(bool writePort) const
{
#ifdef _COMPATIBILITY_1
	return "";
#else
	static char str[22];
	in_addr in;
	in.s_addr = binaryAddress;
	strcpy(str, inet_ntoa( in ));
	if (writePort)
	{
		strcat(str, ":");
#if (defined(__GNUC__)  || defined(__GCCXML__))
		my_itoa(port, str+strlen(str), 10);
#else
		_itoa(port, str+strlen(str), 10);
#endif
	}
	
	return (char*) str;
#endif
}
