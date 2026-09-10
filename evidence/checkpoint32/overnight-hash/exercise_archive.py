"""Run real R5/linked archive methods; no instruction/callee stubs. Independent scalar oracles."""
import argparse,json,struct,sys
from pathlib import Path
sys.path.insert(0,'tools')
from binary import PE,sha
from verify_checkpoint2 import map_symbols,symbol_address
from unicorn import Uc,UC_ARCH_X86,UC_MODE_32,UC_HOOK_CODE
from unicorn.x86_const import *
a=argparse.ArgumentParser();a.add_argument('--run',default='cp32-overnight-hash-archive-three-linked1');a.add_argument('--original-only',action='store_true');args=a.parse_args()
MASK=0xffffffff;DELTA=0x9e3779b9
# Scalar unsigned arithmetic specification, independent of machine register allocation.
def mix(a,b,c):
 for ash,bsh,csh in [(13,8,13),(12,16,5),(3,10,15)]:
  a=((a-b-c)&MASK)^(c>>ash);b=((b-c-a)&MASK)^((a<<bsh)&MASK);c=((c-a-b)&MASK)^(b>>csh)
 return a,b,c

def jenkins(s):
 s=s.split(b'\0')[0];a=b=DELTA;c=0x12345678;n=len(s);i=0
 while n-i>=12:
  a=(a+int.from_bytes(s[i:i+4],'little'))&MASK;b=(b+int.from_bytes(s[i+4:i+8],'little'))&MASK;c=(c+int.from_bytes(s[i+8:i+12],'little'))&MASK;a,b,c=mix(a,b,c);i+=12
 tail=s[i:];a=(a+int.from_bytes(tail[:4],'little'))&MASK;b=(b+int.from_bytes(tail[4:8],'little'))&MASK;c=(c+n+(int.from_bytes(tail[8:],'little')<<8))&MASK
 return mix(a,b,c)[2]

def encrypt(plain,key):
 out=bytearray();k=list(struct.unpack('<4I',key))
 for pos in range(0,len(plain),8):
  v0,v1=struct.unpack('<2I',plain[pos:pos+8]);s=0
  for _ in range(32):
   v0=(v0+((((v1<<4)^(v1>>5))+v1)^((s+k[s&3])&MASK)))&MASK;s=(s+DELTA)&MASK
   v1=(v1+((((v0<<4)^(v0>>5))+v0)^((s+k[(s>>11)&3])&MASK)))&MASK
  out+=struct.pack('<2I',v0,v1);k=[k[0]^v0,k[1]^v1,k[2]^v0,k[3]^v1]
 return bytes(out),struct.pack('<4I',*k)
SAVED={UC_X86_REG_EBX:0x12345678,UC_X86_REG_ESI:0x23456789,UC_X86_REG_EDI:0x3456789a,UC_X86_REG_EBP:0x456789ab}
NAMES={'hash':('?HashString@CArchiveFS@@AAEKPAD@Z',0x64880),'ctor':('??0CTinyEncrypt@@QAE@XZ',0x65a60),'set':('?SetKey@CTinyEncrypt@@QAEXPAEE@Z',0x65ac0),'decrypt':('?DecryptData@CTinyEncrypt@@QAEXKPAE@Z',0x65bc0)}
class Runner:
 def __init__(self,original):
  self.original=original;self.pe=PE('private/samp.dll' if original else Path('build')/args.run/'closure.dll');p=self.pe;self.u=u=Uc(UC_ARCH_X86,UC_MODE_32);u.mem_map(p.base,(p.image_size+4095)&~4095)
  for s in p.sections:u.mem_write(p.base+s['rva'],s['bytes'])
  u.mem_map(0x60000000,0x200000);u.mem_map(0x70000000,0x1000);self.maps={} if original else map_symbols(Path('build')/args.run/'closure.map')
  u.hook_add(UC_HOOK_CODE,lambda uc,at,size,data:uc.emu_stop() if at==0x70000000 else None)
 def call(self,kind,this,*params):
  u=self.u;stack=0x601ff000;u.mem_write(stack,struct.pack('<'+'I'*(1+len(params)),0x70000000,*params));u.reg_write(UC_X86_REG_ESP,stack);u.reg_write(UC_X86_REG_ECX,this)
  for k,v in SAVED.items():u.reg_write(k,v)
  n,r=NAMES[kind];entry=self.pe.base+r if self.original else symbol_address(self.maps,n);u.emu_start(entry,0x70000000,timeout=5000000,count=3000000)
  assert u.reg_read(UC_X86_REG_EIP)==0x70000000;assert u.reg_read(UC_X86_REG_ESP)==stack+4+4*len(params);assert all(u.reg_read(k)==v for k,v in SAVED.items());return u.reg_read(UC_X86_REG_EAX)
results=[];modes=[True] if args.original_only else [True,False]
hashes=[('empty',b''),('embedded_nul',b'abc\0ignored'),('high_bytes',bytes(range(1,256))),('long',b'SAMP'*1024)]+[(f'tail_{n}',bytes(range(65,65+n))) for n in range(1,25)]
for name,value in hashes:
 observed=[]
 for original in modes:
  r=Runner(original);buf=b'\xa5'*32+value+b'\0'+b'\xa5'*32;r.u.mem_write(0x60000fe0,buf);actual=r.call('hash',0x60008000,0x60001000);assert actual==jenkins(value),(name,hex(actual),hex(jenkins(value)));assert bytes(r.u.mem_read(0x60000fe0,len(buf)))==buf;observed.append(actual)
 assert len(set(observed))==1;results.append(dict(case='hash_'+name,result='PASS',answer=observed[0]))
for name,length,key,xor in [('empty',0,bytes(16),0),('short1',1,bytes(16),0),('short3',3,bytes(16),0),('padded4',4,bytes(16),0),('padded7',7,bytes(range(16)),0xaa),('block',8,bytes(16),0),('all_max',8,b'\xff'*16,0),('padded12',12,bytes(range(16)),0),('padded15',15,bytes(range(16)),0xaa),('two_blocks',16,bytes(range(16)),0),('archive_block2048',2048,b'\xff'*16,0xaa)]:
 size=((length//4+1)//2)*8;plain=b'\xff'*size if name=='all_max' else bytes((i*17+3)&255 for i in range(size));effective=bytes(v^xor for v in key);cipher,endkey=encrypt(plain,effective);observed=[]
 for original in modes:
  r=Runner(original);u=r.u;obj=0x60008000;buf=0x60010000;kp=0x60004000;u.mem_write(obj-32,b'\xa5'*80);u.mem_write(kp,key);u.mem_write(buf-32,b'\xa5'*32+cipher+b'\xa5'*32)
  r.call('ctor',obj);r.call('ctor',obj);r.call('set',obj,kp,xor);assert bytes(u.mem_read(obj,16))==effective;assert bytes(u.mem_read(kp,16))==key;r.call('decrypt',obj,length,buf)
  output=bytes(u.mem_read(buf,size));state=bytes(u.mem_read(obj,16));assert output==plain and state==endkey,(name,output.hex(),plain.hex());assert bytes(u.mem_read(buf-32,32))==b'\xa5'*32 and bytes(u.mem_read(buf+size,32))==b'\xa5'*32;assert bytes(u.mem_read(obj-32,32))==b'\xa5'*32 and bytes(u.mem_read(obj+16,32))==b'\xa5'*32;observed.append((sha(output),state.hex()))
 assert len(set(observed))==1;results.append(dict(case='tiny_'+name,result='PASS',declared_length=length,processed_extent=size,ciphertext_sha256=sha(cipher),output_sha256=observed[0][0],final_key=observed[0][1]))
report=dict(result='PASS',run=args.run,mode='original_only' if args.original_only else 'R5_and_linked',cases=results,reference_sha256=sha(Path('private/samp.dll').read_bytes()),scope='Real functions, no instruction or call intercepts. Thiscall stack, nonvolatiles, input/output/key guards verified. Independent scalar Jenkins and modified-XTEA encryption oracles. Historical DecryptData length behavior preserved: floor(length/4) DWORDs rounded up to a pair; padded4/7/12/15 cases provide the full accessible extent. Repeated constructor exercises init-once state. No exhaustive cryptographic validation claim.')
if not args.original_only:report['linked_sha256']=sha((Path('build')/args.run/'closure.dll').read_bytes())
Path('evidence/checkpoint32/overnight-client/archive-boundary-'+('original' if args.original_only else 'linked')+'.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(dict(result=report['result'],mode=report['mode'],cases=len(results))))
