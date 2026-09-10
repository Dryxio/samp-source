"""Exercise real encoder and match finder on both PEs with allocator callbacks only."""
import sys,json,struct,lzma,hashlib
from pathlib import Path
sys.path.insert(0,'tools')
from binary import PE,sha
from verify_checkpoint2 import map_symbols,symbol_address
from unicorn import Uc,UC_ARCH_X86,UC_MODE_32,UC_HOOK_CODE
from unicorn.x86_const import *
RUN='cp32-overnight-lzma-three-linked1';maps=map_symbols(Path('build')/RUN/'closure.map')
SAVED={UC_X86_REG_EBX:0x12345678,UC_X86_REG_ESI:0x23456789,UC_X86_REG_EDI:0x3456789a,UC_X86_REG_EBP:0x456789ab}
cases=[('full',(bytes(range(256))+b'SAMP R5 matching')*3,4096,False),('empty',b'',512,False),('small_output',b'SAMP R5 matching'*30,2,False),('allocation_failure',b'SAMP',128,True)]
results=[]
for name,payload,capacity,fail_alloc in cases:
 pair=[]
 for original in [True,False]:
  pe=PE('private/samp.dll' if original else Path('build')/RUN/'closure.dll');u=Uc(UC_ARCH_X86,UC_MODE_32);u.mem_map(pe.base,(pe.image_size+4095)&~4095)
  for s in pe.sections:u.mem_write(pe.base+s['rva'],s['bytes'])
  u.mem_map(0x60000000,0x2000000);u.mem_map(0x70000000,0x1000)
  STOP,ALLOC,FREE=0x70000000,0x70000100,0x70000200
  SRC,DST,PROP,INFO,HEAP,STACK=0x60001000,0x60100000,0x60002000,0x60003000,0x60300000,0x61fff000
  def put(a,*v):u.mem_write(a,struct.pack('<'+'I'*len(v),*v))
  def word(a):return struct.unpack('<I',u.mem_read(a,4))[0]
  if payload:u.mem_write(SRC,payload)
  u.mem_write(DST,b'\xa5'*(capacity+32));put(INFO,capacity,5,ALLOC,FREE)
  allocations=[];freed=[];heap=[HEAP]
  def hook(uc,at,size,data):
   if at==STOP:uc.emu_stop();return
   if at not in (ALLOC,FREE):return
   sp=uc.reg_read(UC_X86_REG_ESP);assert word(sp+4)==INFO+8
   if at==ALLOC:
    n=word(sp+8);assert n<0x1000000;allocations.append(n);v=0 if fail_alloc else heap[0];heap[0]+=(n+15)&~15;assert heap[0]<0x61f00000
   else:freed.append(word(sp+8));v=0
   uc.reg_write(UC_X86_REG_EAX,v);uc.reg_write(UC_X86_REG_EIP,word(sp));uc.reg_write(UC_X86_REG_ESP,sp+4)
  u.hook_add(UC_HOOK_CODE,hook)
  def call(entry,args):
   put(STACK,STOP,*args);u.reg_write(UC_X86_REG_ESP,STACK)
   for r,v in SAVED.items():u.reg_write(r,v)
   u.emu_start(entry,STOP,timeout=10000000,count=10000000)
   assert u.reg_read(UC_X86_REG_EIP)==STOP,'call did not return'
   assert u.reg_read(UC_X86_REG_ESP)==STACK+4 and all(u.reg_read(r)==v for r,v in SAVED.items()),'ABI changed'
   return u.reg_read(UC_X86_REG_EAX)
  call(pe.base+0xc0180 if original else symbol_address(maps,'_LzmaEncProps_Init'),[PROP])
  put(PROP+4,65536);put(PROP+52,1)
  ret=call(pe.base+0xc38e0 if original else symbol_address(maps,'_LzmaEncode'),[DST,INFO,SRC,len(payload),PROP,PROP+0x100,INFO+4,1,0,INFO+8,INFO+8])
  assert bytes(u.mem_read(DST+capacity,32))==b'\xa5'*32
  n=word(INFO);assert n<=capacity
  out=bytes(u.mem_read(DST,n));props=bytes(u.mem_read(PROP+0x100,5))
  if name in ('full','empty'):
   assert ret==0
   assert lzma.decompress(out,format=lzma.FORMAT_RAW,filters=[dict(id=lzma.FILTER_LZMA1,dict_size=65536,lc=3,lp=0,pb=2)])==payload
  else:assert ret!=0
  pair.append(dict(return_code=ret,output_size=n,props=props.hex(),output_sha256=hashlib.sha256(out).hexdigest(),allocation_sizes=allocations,free_count=len(freed)))
 assert pair[0]==pair[1],(name,pair)
 results.append(dict(case=name,result='PASS',observed=pair[0],stack_and_nonvolatile_preserved=True,output_guard_preserved=True))
report=dict(result='PASS',run=RUN,reference_sha256=sha(Path('private/samp.dll').read_bytes()),linked_sha256=sha((Path('build')/RUN/'closure.dll').read_bytes()),cases=results,scope='Real LzmaEncode, internal encoder and match finder run on original and linked PEs. Only caller-supplied allocation/free callbacks intercepted; __aullshr runtime executes normally. Successful streams independently decoded by Python liblzma. Empty, insufficient output and allocation failure included. Not exhaustive.')
Path('build/overnight/lzma/encoder-boundary-abi.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report,indent=2))
