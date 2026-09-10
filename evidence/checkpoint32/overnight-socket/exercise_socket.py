"""Execute genuine original/linked SocketLayer, codec and CRT; four OS API shims only."""
import sys,json,struct,hashlib,ipaddress
from pathlib import Path
sys.path.insert(0,'tools')
from binary import PE,sha
from verify_checkpoint2 import map_symbols,symbol_address,import_slots
from unicorn import Uc,UC_ARCH_X86,UC_MODE_32,UC_HOOK_CODE
from unicorn.x86_const import *
RUN=sys.argv[1] if len(sys.argv)>1 else 'cp32-overnight-socket-one-linked1'
folder=Path('build')/RUN;maps=map_symbols(folder/'closure.map');ref=PE('private/samp.dll');table=ref.read(0xfe500,256)
SAVED={UC_X86_REG_EBX:0x12345678,UC_X86_REG_ESI:0x23456789,UC_X86_REG_EDI:0x3456789a,UC_X86_REG_EBP:0x456789ab}
cases=[dict(name='invalid_socket',payload=b'ABC',sock=0xffffffff),dict(name='empty',payload=b''),dict(name='all_byte_values',payload=bytes(range(256))),dict(name='mtu_payload',payload=bytes(range(256))*5+b'MTU'*70),dict(name='whole_buffer_boundary',payload=(bytes(range(256))*128)[:32767]),dict(name='zero_send_retry',payload=b'retry',returns=[0,6]),dict(name='connection_reset',payload=b'reset',returns=[0xffffffff],error=10054),dict(name='would_block',payload=b'blocked',returns=[0xffffffff],error=10035),dict(name='other_error',payload=b'error',returns=[0xffffffff],error=10022),dict(name='host_overload',payload=b'127.0.0.1',host='127.0.0.1'),dict(name='host_invalid_address',payload=b'invalid',host='invalid'),dict(name='host_invalid_socket',payload=b'ABC',host='192.0.2.1',sock=0xffffffff)]
results=[]
for case in cases:
 pair=[]
 payload=case['payload'];port=case.get('port',7777);sock=case.get('sock',0x1234);host=case.get('host');address=bytes([192,0,2,15])
 if host is not None:
  try:address=ipaddress.IPv4Address(host).packed
  except ipaddress.AddressValueError:address=b'\xff'*4
 checksum=0
 for b in payload:checksum^=b&0xaa
 key=(port^0xcccc)&255
 expected=bytes([checksum])+bytes(table[b]^(key if i&1 else 0) for i,b in enumerate(payload))
 for original in [True,False]:
  pe=ref if original else PE(folder/'closure.dll');u=Uc(UC_ARCH_X86,UC_MODE_32);u.mem_map(pe.base,(pe.image_size+4095)&~4095)
  for sec in pe.sections:
   if sec['bytes']:u.mem_write(pe.base+sec['rva'],sec['bytes'])
  u.mem_map(0x60000000,0x400000);u.mem_map(0x70000000,0x1000)
  STOP,HTONS,INET,SEND,ERROR=0x70000000,0x70000100,0x70000200,0x70000300,0x70000400
  SRC,HOST,STACK=0x60001000,0x60010000,0x603fe000
  def put(a,*values):u.mem_write(a,struct.pack('<'+'I'*len(values),*(v&0xffffffff for v in values)))
  def word(a):return struct.unpack('<I',u.mem_read(a,4))[0]
  def addr(n,rva):return pe.base+rva if original else symbol_address(maps,n)
  buffer=addr('?r5DatagramOutput@@3PAEA',0x119a38);instance=addr('?I@SocketLayer@@0V1@A',0x121a39)
  imports=import_slots(pe);original_slots={symbol:next(at for at,value in imports.items() if value==('wsock32.dll',ordinal)) for symbol,ordinal in [('__imp__htons@4','#9'),('__imp__inet_addr@4','#10'),('__imp__sendto@24','#20'),('__imp__WSAGetLastError@0','#111')]}
  for symbol,hook in [('__imp__htons@4',HTONS),('__imp__inet_addr@4',INET),('__imp__sendto@24',SEND),('__imp__WSAGetLastError@0',ERROR)]:
   slot=original_slots[symbol] if original else symbol_address(maps,symbol);assert slot in imports;put(slot,hook)
  u.mem_write(SRC-16,b'\x5a'*16+payload+b'\x5a'*16);u.mem_write(HOST,(host or '').encode()+b'\0')
  u.mem_write(buffer-16,b'\xa5'*(32768+32));trace=[];send_calls=[];returns=list(case.get('returns',[len(expected)]));error_calls=[0]
  def hook(uc,at,size,data):
   sp=uc.reg_read(UC_X86_REG_ESP);ret=word(sp)
   if at==HTONS:
    value=word(sp+4)&65535;assert value==port;answer=((value&255)<<8)|(value>>8);argc=1;trace.append('htons')
   elif at==INET:
    ptr=word(sp+4);raw=bytearray()
    while len(raw)<64 and bytes(uc.mem_read(ptr+len(raw),1))!=b'\0':raw.extend(uc.mem_read(ptr+len(raw),1))
    assert raw.decode()==host;answer=int.from_bytes(address,'little');argc=1;trace.append('inet_addr')
   elif at==SEND:
    args=[word(sp+4+i*4) for i in range(6)];s,b,n,flags,sa,salen=args
    assert s==sock and b==buffer and n==len(expected) and flags==0 and salen==16
    actual=bytes(uc.mem_read(b,n));assert actual==expected,'packet encoding differs from independent algorithm'
    sockaddr=bytes(uc.mem_read(sa,16));assert sockaddr[:2]==b'\x02\x00' and sockaddr[2:4]==port.to_bytes(2,'big') and sockaddr[4:8]==address
    assert bytes(uc.mem_read(buffer-16,16))==b'\xa5'*16 and bytes(uc.mem_read(buffer+32768,16))==b'\xa5'*16
    send_calls.append(dict(length=n,encoded_sha256=hashlib.sha256(actual).hexdigest(),sockaddr=sockaddr.hex()))
    assert len(send_calls)<=len(returns),'unexpected retry';answer=returns[len(send_calls)-1];argc=6;trace.append('sendto')
   elif at==ERROR:
    error_calls[0]+=1;answer=case.get('error',0);argc=0;trace.append('WSAGetLastError')
   else:raise AssertionError(('unexpected shim',hex(at)))
   uc.reg_write(UC_X86_REG_EAX,answer);uc.reg_write(UC_X86_REG_ESP,sp+4*(argc+1));uc.reg_write(UC_X86_REG_EIP,ret)
  u.hook_add(UC_HOOK_CODE,hook,begin=HTONS,end=ERROR)
  def call(entry,args,callee_pops,ecx=0):
   u.mem_write(STACK-0x1000,b'\xcc'*0x2000);put(STACK,STOP,*args);u.reg_write(UC_X86_REG_ESP,STACK);u.reg_write(UC_X86_REG_ECX,ecx)
   for reg,val in SAVED.items():u.reg_write(reg,val)
   u.emu_start(entry,STOP,timeout=10000000,count=10000000)
   assert u.reg_read(UC_X86_REG_EIP)==STOP,'body did not return'
   assert u.reg_read(UC_X86_REG_ESP)==STACK+4+callee_pops,'callee argument cleanup differs'
   assert all(u.reg_read(r)==v for r,v in SAVED.items()),'nonvolatile register changed'
   return u.reg_read(UC_X86_REG_EAX)
  call(addr('?R5SetDatagramPort@@YAXI@Z',0x1f710),[port],0)
  numeric=host is None;entry=addr('?SendTo@SocketLayer@@QAEHIPBDHIG@Z' if numeric else '?SendTo@SocketLayer@@QAEHIPBDHQADG@Z',0x53ab0 if numeric else 0x53b40)
  rc=call(entry,[sock,SRC,len(payload),int.from_bytes(address,'little') if numeric else HOST,port],20,instance)
  invalid=sock==0xffffffff;want_return=0xffffffff if invalid else case.get('error',0)
  assert rc==want_return
  assert len(send_calls)==(0 if invalid else len(returns));assert error_calls[0]==(1 if not invalid and returns[-1]==0xffffffff else 0)
  assert bytes(u.mem_read(SRC-16,len(payload)+32))==b'\x5a'*16+payload+b'\x5a'*16
  written=0 if invalid else len(expected);assert bytes(u.mem_read(buffer+written,32768-written))==b'\xa5'*(32768-written)
  assert bytes(u.mem_read(buffer-16,16))==b'\xa5'*16 and bytes(u.mem_read(buffer+32768,16))==b'\xa5'*16
  pair.append(dict(return_code=rc,imports=trace,send_calls=send_calls,WSAGetLastError_calls=error_calls[0],encoded_bytes=written))
 assert pair[0]==pair[1],(case['name'],pair)
 results.append(dict(case=case['name'],result='PASS',payload_length=len(payload),observed=pair[0],stack_nonvolatile_input_and_buffer_guards_preserved=True))
report=dict(status='PASS',run=RUN,reference_sha256=sha(ref.data),linked_sha256=sha((folder/'closure.dll').read_bytes()),test_source_sha256=sha(Path(__file__).read_bytes()),cases=results,scope='Execute genuine complete SendTo numeric and host overloads, R5SetDatagramPort, datagram checksum/substitution/encoding and called CRT on original and linked PEs. Four explicitly authorized Windows API shims only: htons, inet_addr, sendto, WSAGetLastError. API stdcall cleanup validated by whole-call ABI; deterministic argument/results checked. Codec outputs additionally checked by independent Python algorithm. Boundaries include all32768 output bytes, exact canaries, input preservation, retry-on-zero and three errors. No actual OS/network execution; not exhaustive.')
Path('evidence/checkpoint32/overnight-network/socket-boundary-abi.json').write_text(json.dumps(report,indent=2)+'\n');print('PASS',len(results),'paired cases including whole32768 buffer and both overloads')
