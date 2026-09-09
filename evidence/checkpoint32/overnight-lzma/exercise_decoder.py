"""Compare genuine R5 and linked LZMA decoder; intercept allocator callbacks only."""
import sys,json,struct,lzma,hashlib
from pathlib import Path
sys.path.insert(0,'tools')
from binary import PE,sha
from verify_checkpoint2 import map_symbols,symbol_address
from unicorn import Uc,UC_ARCH_X86,UC_MODE_32,UC_HOOK_CODE
from unicorn.x86_const import *
RUN='cp32-overnight-lzma-three-linked1';maps=map_symbols(Path('build')/RUN/'closure.map')
payload=(bytes(range(256))+b'SAMP R5 source matching\x00')*4
filters=[dict(id=lzma.FILTER_LZMA1,dict_size=65536,lc=3,lp=0,pb=2)]
compressed=lzma.compress(payload,format=lzma.FORMAT_RAW,filters=filters)
props=bytes.fromhex('5d00000100');empty=lzma.compress(b'',format=lzma.FORMAT_RAW,filters=filters)
cases=[('full',compressed,props,len(payload),1,False),('empty',empty,props,0,1,False),('partial_output',compressed,props,len(payload)//2,0,False),('truncated_input',compressed[:len(compressed)//2],props,len(payload),1,False),('bad_properties',compressed,b'\xff'+props[1:],len(payload),1,False),('allocation_failure',compressed,props,len(payload),1,True)]
SAVED={UC_X86_REG_EBX:0x12345678,UC_X86_REG_ESI:0x23456789,UC_X86_REG_EDI:0x3456789a,UC_X86_REG_EBP:0x456789ab}
results=[]
for name,src,prop,outsize,finish,fail_alloc in cases:
 pair=[]
 for original in [True,False]:
  pe=PE('private/samp.dll' if original else Path('build')/RUN/'closure.dll')
  u=Uc(UC_ARCH_X86,UC_MODE_32);u.mem_map(pe.base,(pe.image_size+4095)&~4095)
  for s in pe.sections:u.mem_write(pe.base+s['rva'],s['bytes'])
  u.mem_map(0x60000000,0x800000);u.mem_map(0x70000000,0x1000)
  STOP,ALLOC,FREE=0x70000000,0x70000100,0x70000200
  SRC,DST,PROP,INFO,HEAP,STACK=0x60001000,0x60100000,0x60002000,0x60003000,0x60300000,0x607ff000
  def put(a,*v):u.mem_write(a,struct.pack('<'+'I'*len(v),*v))
  def word(a):return struct.unpack('<I',u.mem_read(a,4))[0]
  u.mem_write(SRC,src);u.mem_write(PROP,prop);u.mem_write(DST,b'\xa5'*(outsize+32))
  put(INFO,outsize,len(src),0xcccccccc,ALLOC,FREE)
  calls=[];frees=[];heap=[HEAP]
  def hook(uc,at,size,data):
   if at==STOP:uc.emu_stop();return
   if at not in (ALLOC,FREE):return
   sp=uc.reg_read(UC_X86_REG_ESP);assert word(sp+4)==INFO+12,'wrong allocator interface pointer'
   if at==ALLOC:
    n=word(sp+8);assert n<0x100000;calls.append(n)
    value=0 if fail_alloc else heap[0];heap[0]+=(n+15)&~15
   else:
    frees.append(word(sp+8));value=0
   uc.reg_write(UC_X86_REG_EAX,value);uc.reg_write(UC_X86_REG_EIP,word(sp));uc.reg_write(UC_X86_REG_ESP,sp+4)
  u.hook_add(UC_HOOK_CODE,hook)
  entry=pe.base+0xc00a0 if original else symbol_address(maps,'_LzmaDecode')
  put(STACK,STOP,DST,INFO,SRC,INFO+4,PROP,len(prop),finish,INFO+8,INFO+12)
  u.reg_write(UC_X86_REG_ESP,STACK)
  for reg,val in SAVED.items():u.reg_write(reg,val)
  u.emu_start(entry,STOP,timeout=5000000,count=3000000)
  assert u.reg_read(UC_X86_REG_EIP)==STOP,'decoder did not return'
  assert u.reg_read(UC_X86_REG_ESP)==STACK+4,'cdecl stack mismatch'
  assert all(u.reg_read(k)==v for k,v in SAVED.items()),'nonvolatile registers changed'
  assert bytes(u.mem_read(DST+outsize,32))==b'\xa5'*32,'output boundary changed'
  n=word(INFO);assert n<=outsize
  output=bytes(u.mem_read(DST,n));ret=u.reg_read(UC_X86_REG_EAX)
  if name in ('full','partial_output'):assert ret==0 and output==payload[:outsize]
  if name=='empty':assert ret==0 and n==0
  if name in ('bad_properties','allocation_failure'):assert ret!=0
  pair.append(dict(return_code=ret,dest_len=n,src_len=word(INFO+4),status=word(INFO+8),output_sha256=hashlib.sha256(output).hexdigest(),allocation_sizes=calls,free_count=len(frees)))
 assert pair[0]==pair[1],(name,pair)
 results.append(dict(case=name,result='PASS',observed=pair[0],stack_and_nonvolatile_preserved=True,output_guard_preserved=True))
report=dict(result='PASS',run=RUN,reference_sha256=sha(Path('private/samp.dll').read_bytes()),linked_sha256=sha((Path('build')/RUN/'closure.dll').read_bytes()),cases=results,scope='Real complete LzmaDecode and internal routines run in both PEs. Only caller-supplied ISzAlloc allocation/free callbacks intercepted. Known raw LZMA stream plus empty, output limit, truncated input, invalid properties and allocation failure. No claim of exhaustive codec validation.')
Path('build/overnight/lzma/decoder-boundary-abi.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report,indent=2))
