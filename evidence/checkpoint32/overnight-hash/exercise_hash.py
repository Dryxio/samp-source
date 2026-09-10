"""Real original and linked digest routines, no function or API shims."""
import sys,json,struct,hashlib
from pathlib import Path
sys.path.insert(0,'tools')
from binary import PE,sha
from verify_checkpoint2 import map_symbols,symbol_address
from unicorn import Uc,UC_ARCH_X86,UC_MODE_32
from unicorn.x86_const import *
run=sys.argv[1] if len(sys.argv)>1 else 'cp32-overnight-hash-archive-two-linked1'
folder=Path('build')/run;maps=map_symbols(folder/'closure.map');m=json.load(open('evidence/checkpoint32/overnight-hash/hash-reviewed-manifest.json'));rvas={r['anchor']:r['rva'] for r in m['regions']}
saved={UC_X86_REG_EBX:0x12345678,UC_X86_REG_ESI:0x23456789,UC_X86_REG_EDI:0x3456789a,UC_X86_REG_EBP:0x456789ab}
results=[]
for data in [b'',b'abc',b'a'*55,b'b'*56,b'c'*63,b'd'*64,b'e'*65,bytes(range(256))*16]:
 pair=[]
 for original in [True,False]:
  pe=PE('private/samp.dll' if original else folder/'closure.dll');u=Uc(UC_ARCH_X86,UC_MODE_32);u.mem_map(pe.base,(pe.image_size+4095)&~4095)
  for sec in pe.sections:
   if sec['bytes']:u.mem_write(pe.base+sec['rva'],sec['bytes'])
  u.mem_map(0x60000000,0x400000);u.mem_map(0x70000000,4096)
  ctx,src,out,stack,stop=0x60001000,0x60010000,0x60003000,0x603fe000,0x70000000
  u.mem_write(src,data or b'\0')
  def call(name,args,pops=0,this=0):
   u.mem_write(stack-4096,b'\xcc'*8192);u.mem_write(stack,struct.pack('<'+'I'*(len(args)+1),stop,*args));u.reg_write(UC_X86_REG_ESP,stack);u.reg_write(UC_X86_REG_ECX,this)
   for reg,val in saved.items():u.reg_write(reg,val)
   entry=pe.base+rvas[name] if original else symbol_address(maps,name)
   u.emu_start(entry,stop,timeout=10000000,count=10000000)
   assert u.reg_read(UC_X86_REG_EIP)==stop and u.reg_read(UC_X86_REG_ESP)==stack+4+pops
   assert all(u.reg_read(reg)==val for reg,val in saved.items())
   return u.reg_read(UC_X86_REG_EAX)
  digests={}
  for alg in ['md5','sha1']:
   size=88 if alg=='md5' else 102
   u.mem_write(ctx-16,b'\xa5'*(size+32));u.mem_write(out-16,b'\x5a'*64)
   if alg=='md5':
    call('?MD5Init@@YAXPAUMD5Context@@@Z',[ctx])
    for off in range(0,len(data),17):call('?MD5Update@@YAXPAUMD5Context@@PBEI@Z',[ctx,src+off,min(17,len(data)-off)])
    call('?MD5Final@@YAXQAEPAUMD5Context@@@Z',[out,ctx]);digest=bytes(u.mem_read(out,16)).hex();n=16
   else:
    call('?Reset@SHA1@fhicl@@QAEXXZ',[],this=ctx)
    for off in range(0,len(data),17):call('?Input@SHA1@fhicl@@QAEXPBEI@Z',[src+off,min(17,len(data)-off)],8,ctx)
    assert call('?Result@SHA1@fhicl@@QAE_NPAI@Z',[out],4,ctx)&255==1
    digest=b''.join(v.to_bytes(4,'big') for v in struct.unpack('<5I',u.mem_read(out,20))).hex();n=20
    assert call('?Result@SHA1@fhicl@@QAE_NPAI@Z',[out],4,ctx)&255==1
   assert digest==getattr(hashlib,alg)(data).hexdigest(),(alg,len(data),digest)
   assert bytes(u.mem_read(ctx-16,16))==b'\xa5'*16 and bytes(u.mem_read(ctx+size,16))==b'\xa5'*16
   assert bytes(u.mem_read(out-16,16))==b'\x5a'*16 and bytes(u.mem_read(out+n,16))==b'\x5a'*16
   digests[alg]=digest
  assert bytes(u.mem_read(src,len(data)))==data
  pair.append(digests)
 assert pair[0]==pair[1];results.append(dict(length=len(data),input_sha256=hashlib.sha256(data).hexdigest(),digests=pair[0]))
result=dict(status='PASS',run=run,reference_sha256=sha(Path('private/samp.dll').read_bytes()),linked_sha256=sha((folder/'closure.dll').read_bytes()),cases=results,scope='Real original and linked MD5/SHA1 plus real CRT, no shims. Empty, block/padding boundaries, 4096 bytes streamed by17; hashlib independent answers; cdecl/thiscall stack cleanup and nonvolatiles; input and output/context guards.')
Path('evidence/checkpoint32/overnight-hash/hash-boundary-abi.json').write_text(json.dumps(result,indent=2)+'\n');print('PASS',len(results),'paired cases for both digests')
