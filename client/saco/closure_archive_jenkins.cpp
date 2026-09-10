// Original ArchiveFS Jenkins method, declaration-only class; never instantiated here.
#include <windows.h>
#include <string.h>
#define OBFUSCATE_KEY		0xC103D3E7
#define OBFUSCATE_DATA(a)	((((a) << 19) | ((a) >> 13)) ^ OBFUSCATE_KEY)
#define UNOBFUSCATE_DATA(a) ((((a) ^ OBFUSCATE_KEY) >> 19) | (((a) ^ OBFUSCATE_KEY) << 13))
class CArchiveFS { static DWORD ms_dwHashInit; DWORD HashString(PCHAR); };
DWORD CArchiveFS::ms_dwHashInit = OBFUSCATE_DATA(0x9E3779B9);
DWORD CArchiveFS::HashString(PCHAR szString)
{
	// This is an implementation of the Jenkins hash

#	define mix(a,b,c) \
	{ \
		a -= b; a -= c; a ^= (c>>13); \
		b -= c; b -= a; b ^= (a<<8); \
		c -= a; c -= b; c ^= (b>>13); \
		a -= b; a -= c; a ^= (c>>12);  \
		b -= c; b -= a; b ^= (a<<16); \
		c -= a; c -= b; c ^= (b>>5); \
		a -= b; a -= c; a ^= (c>>3);  \
		b -= c; b -= a; b ^= (a<<10); \
		c -= a; c -= b; c ^= (b>>15); \
	}

	register BYTE* k = (BYTE*)szString;
	register DWORD initval = 0x12345678;
	register DWORD length;

	length = (DWORD)strlen(szString);

	register DWORD a,b,c,len;

	/* Set up the internal state */
	len = length;
	a = b = UNOBFUSCATE_DATA(ms_dwHashInit); /* the golden ratio; an arbitrary value */
	c = initval;         /* the previous hash value */

	/*---------------------------------------- handle most of the key */
	while (len >= 12)
	{
	a += (k[0] +((DWORD)k[1]<<8) +((DWORD)k[2]<<16) +((DWORD)k[3]<<24));
	b += (k[4] +((DWORD)k[5]<<8) +((DWORD)k[6]<<16) +((DWORD)k[7]<<24));
	c += (k[8] +((DWORD)k[9]<<8) +((DWORD)k[10]<<16)+((DWORD)k[11]<<24));
	mix(a,b,c);
	k += 12; len -= 12;
	}

	/*------------------------------------- handle the last 11 bytes */
	c += length;
	switch(len)              /* all the case statements fall through */
	{
		case 11: c+=((DWORD)k[10]<<24);
		case 10: c+=((DWORD)k[9]<<16);
		case 9 : c+=((DWORD)k[8]<<8);
		  /* the first byte of c is reserved for the length */
		case 8 : b+=((DWORD)k[7]<<24);
		case 7 : b+=((DWORD)k[6]<<16);
		case 6 : b+=((DWORD)k[5]<<8);
		case 5 : b+=k[4];
		case 4 : a+=((DWORD)k[3]<<24);
		case 3 : a+=((DWORD)k[2]<<16);
		case 2 : a+=((DWORD)k[1]<<8);
		case 1 : a+=k[0];
		 /* case 0: nothing left to add */
	}
	mix(a,b,c);
	/*-------------------------------------------- report the result */
	return c;
}

