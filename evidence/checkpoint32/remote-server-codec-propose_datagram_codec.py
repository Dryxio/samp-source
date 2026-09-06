from pathlib import Path
import re,json,hashlib
b=Path('build/agent-independent');ref=Path('/Users/salimtrouve/Documents/GitHub/samp-r5-preservation/references/michael-fa-samp/raknet/SAMP/samp_netencr.cpp');s=ref.read_text();v=s.split('unsigned char sampEncrTable[256] =')[1].split('};')[0];table=bytes(int(x,16) for x in re.findall(r'0x([0-9A-Fa-f]+)',v));assert len(table)==256
code='''// Source025 datagram checksum/substitution adapted to independently identified R5 helper ABIs.
#include <string.h>
extern unsigned char r5DatagramSubstitution[256];
extern unsigned short r5DatagramKey;
unsigned char R5DatagramChecksum(const unsigned char *data, unsigned length)
{
    unsigned char checksum=0;
    for(unsigned i=0; i<length; ++i)
        checksum ^= data[i] & 0xAA;
    return checksum;
}
void R5DatagramSubstitute(unsigned char firstKey, unsigned char secondKey, unsigned char *data, unsigned length)
{
    int alternate=0;
    for(unsigned i=0; i<length; ++i)
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
'''
(b/'eval_datagram_codec.cpp').write_text(code)
(b/'eval_store_datagram_key.cpp').write_text('// Real WORD key owner, written by R5SetDatagramPort; original118B74.\nunsigned short r5DatagramKey;\n')
(b/'eval_store_datagram_table.cpp').write_text('// Complete substitution data from source025, exact original FE500/256.\nunsigned char r5DatagramSubstitution[256] =\n{\n'+',\n'.join('    '+', '.join('0x%02X'%x for x in table[i:i+16]) for i in range(0,256,16))+'\n};\n')
p=Path('vendor/upstream/raknet/SocketLayer.cpp');src=p.read_text();st=src.index('int SocketLayer::SendTo( SOCKET s, const char *data, int length, unsigned int binaryAddress');en=src.index('int SocketLayer::SendTo(',st+1);body=src[st:en];body=body[:body.rfind('}')+1];body=body.replace('\n\tdo\n','\n\tR5DatagramEncode(r5DatagramOutput, (const unsigned char *)data, (unsigned *)&length);\n\tdo\n').replace('sendto( s, data, length,','sendto( s, (const char *)r5DatagramOutput, length,')
(b/'eval_socket_datagram_send.cpp').write_text('// Original SocketLayer SendTo with the R5 packet transformation inserted at proven callsite53AF5.\n#include "../raknet/SocketLayer.h"\nextern unsigned char r5DatagramOutput[];\nvoid R5DatagramEncode(unsigned char *, const unsigned char *, unsigned *);\n'+body+'\n')
j=dict(status='PROPOSAL_NOT_ACCEPTED',roots=[dict(name=n,rva=r,size=z) for n,r,z in [('R5DatagramChecksum',0x1f630,34),('R5DatagramSubstitute',0x1f660,74),('R5DatagramEncode',0x1f6b0,86),('R5SetDatagramPort',0x1f710,16),('SocketLayer::SendTo(unsigned)',0x53ab0,137)]],source025=dict(path=str(ref),sha256=hashlib.sha256(ref.read_bytes()).hexdigest(),table_exact_bytes=256,adaptations=['Original025 single checksum/memcpy/substitution routine split into actual R5 helpers34/74/86.','R5 uses two alternating byte keys; first0, second low byte of globalWORD set(port XOR CCCC). 025 uses direct port XOR CC and parity.','R5 output and length pointer arguments replace025 globalbuffer+length value.']),owners=dict(table=dict(rva=0xfe500,size=256,source_backed=True,whole_sha256=hashlib.sha256(table).hexdigest()),key=dict(rva=0x118b74,size=2,writer_rva=0x1f710,reader_rva=0x1f6b0,initial_zero=True),buffer=dict(rva=0x119a38,status='IDENTIFIED_ADDRESS_BUT_R5_FULL_EXTENT_UNPROVEN',source025_declared_size=4092,warning='No buffer storage unit supplied. Do not accept SendTo until actual whole R5 owner extent established; codec210 can be accepted independently.')),scope=['Normal C++ only; data table from fullsource reference, no instruction-byte source.','No runtime network execution; source-only proposal.','Key setter callerB930/siteB9CD independently identified.','Source buffer4092 is a reference hint, not a proven R5 allocation.'],files=[dict(path=str(b/n),sha256=hashlib.sha256((b/n).read_bytes()).hexdigest()) for n in ['eval_datagram_codec.cpp','eval_store_datagram_key.cpp','eval_store_datagram_table.cpp','eval_socket_datagram_send.cpp']])
(b/'datagram-codec-proposal.json').write_text(json.dumps(j,indent=2)+'\n')
print('stable codec210, Send137 pending buffer extent')
