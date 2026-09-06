// Source025 datagram checksum/substitution adapted to independently identified R5 helper ABIs.
#include <string.h>
extern unsigned char r5DatagramSubstitution[256];
extern unsigned short r5DatagramKey;
unsigned char R5DatagramChecksum(const unsigned char *data, unsigned length)
{
    unsigned char checksum=0;
    for(unsigned i=0; i!=length; ++i)
        checksum ^= data[i] & 0xAA;
    return checksum;
}
void R5DatagramSubstitute(unsigned char firstKey, unsigned char secondKey, unsigned char *data, unsigned length)
{
    int alternate=0;
    for(unsigned i=0; i!=length; ++i)
    {
        data[i]=r5DatagramSubstitution[data[i]];
        if(alternate==0)
        {
            data[i]^=firstKey;
            alternate=1;
        }
        else
        {
            data[i]^=secondKey;
            --alternate;
        }
    }
}
void R5DatagramEncode(unsigned char *output, const unsigned char *input, unsigned *length)
{
    unsigned char key=(unsigned char)r5DatagramKey;
    output[0]=R5DatagramChecksum(input,*length);
    memcpy(output+1,input,*length);
    R5DatagramSubstitute(0,key,output+1,*length);
    ++*length;
}
void R5SetDatagramPort(unsigned port)
{
    r5DatagramKey=(unsigned short)(port^0xCCCC);
}
