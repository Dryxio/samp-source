"""Execute reconstructed R5 chains; intercept only the external GTA engine.

Reference and independently linked images run under the same ABI fixtures.
No R5 function, including GamePool_Ped_GetAt, is replaced by a test provider.
"""
import struct,sys
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[2]/'tools'))
from binary import need
from verify_checkpoint2 import symbol_address
from unicorn import Uc,UC_ARCH_X86,UC_MODE_32,UC_HOOK_CODE
from unicorn.x86_const import *

ACTOR,PED,VTABLE,POOL=0x60000100,0x60001000,0x60002000,0x60003000
STOP,NATIVE_DTOR,STACK=0x70000000,0x70001000,0x60008000
PRESERVED={UC_X86_REG_EBX:0x12345678,UC_X86_REG_ESI:0x23456789,UC_X86_REG_EDI:0x3456789a,UC_X86_REG_EBP:0x456789ab}

def exercise(gate):
    cases=[]
    for reference in (True,False):
        pe=gate.reference if reference else gate.linked
        addr=lambda n: next(pe.base+r['rva'] for r in gate.contract['regions'] if r['symbol'].startswith('?'+n+'@')) if reference else next(symbol_address(gate.maps,r['symbol']) for r in gate.contract['regions'] if r['symbol'].startswith('?'+n+'@'))
        ranges=[(addr(r['symbol'].split('@')[0][1:]),r['size']) for r in gate.contract['regions']]
        for scenario in ('null','missing','placeable','live','remove-null','remove-placeable','remove-absent','remove-live'):
            uc=Uc(UC_ARCH_X86,UC_MODE_32)
            uc.mem_map(pe.base,(pe.image_size+4095)&~4095)
            for s in pe.sections:uc.mem_write(pe.base+s['rva'],s['bytes'])
            for base,size in [(0x400000,0x5000),(0x560000,0x10000),(0xb74000,0x1000),(0x60000000,0x10000),(STOP,0x2000)]:uc.mem_map(base,size)
            def write(a,v):uc.mem_write(a,struct.pack('<I',v))
            def read(a):return struct.unpack('<I',uc.mem_read(a,4))[0]
            write(0xb74490,POOL);write(ACTOR+0x40,PED);write(ACTOR+0x44,0x1234);write(ACTOR+0x48,PED)
            write(PED,VTABLE);write(VTABLE,NATIVE_DTOR);write(PED+0xb4,1)
            if scenario=='null':write(ACTOR+0x48,0)
            if scenario in ('placeable','remove-placeable'):write(PED,0x863c40)
            if scenario=='remove-null':write(ACTOR+0x40,0)
            if scenario=='remove-absent':write(PED+0xb4,0)
            write(STACK,STOP);uc.reg_write(UC_X86_REG_ESP,STACK);uc.reg_write(UC_X86_REG_ECX,ACTOR)
            for reg,value in PRESERVED.items():uc.reg_write(reg,value)
            calls=[];seen=set()
            def hook(m,a,size,user):
                seen.add(a)
                if a in (0x404910,NATIVE_DTOR,0x563280):
                    sp=m.reg_read(UC_X86_REG_ESP);ret=read(sp);arg=read(sp+4)
                    if a==0x404910:
                        need(m.reg_read(UC_X86_REG_ECX)==POOL and arg==0x1234,'GTA pool ABI differs')
                        m.reg_write(UC_X86_REG_EAX,0 if scenario=='missing' else PED);cleanup=8
                    elif a==NATIVE_DTOR:
                        need(m.reg_read(UC_X86_REG_ECX)==PED and arg==1,'GTA virtual destructor ABI differs');cleanup=8
                    else:
                        need(arg==PED,'GTA world remove argument differs');cleanup=4
                    calls.append(a);m.reg_write(UC_X86_REG_ESP,sp+cleanup);m.reg_write(UC_X86_REG_EIP,ret)
                else:need(any(start<=a<start+length for start,length in ranges),'execution escaped accepted bodies')
            uc.hook_add(UC_HOOK_CODE,hook)
            entry=addr('Remove' if scenario.startswith('remove-') else 'Destroy')
            uc.emu_start(entry,STOP,count=2000)
            need(uc.reg_read(UC_X86_REG_EIP)==STOP,'did not return')
            need(uc.reg_read(UC_X86_REG_ESP)==STACK+4,'thiscall stack differs')
            for reg,value in PRESERVED.items():need(uc.reg_read(reg)==value,'callee-saved register corrupted')
            if scenario.startswith('remove-'):
                need(calls==([0x563280] if scenario=='remove-live' else []),'wrong removal path')
                if scenario=='remove-live':need(addr('WorldRemoveEntity') in seen,'R5 world bridge bypassed')
            else:
                expected=[] if scenario=='null' else [0x404910]
                if scenario=='live':expected.append(NATIVE_DTOR)
                need(calls==expected,'wrong destruction path')
                need(read(ACTOR+0x40)==0 and read(ACTOR+0x48)==0,'actor pointers not cleared')
                need(read(ACTOR+0x44)==(0x1234 if scenario=='live' else 0),'original GTA id semantics lost')
                if scenario!='null':need(addr('GamePool_Ped_GetAt') in seen,'R5 pool bridge bypassed')
            cases.append(dict(image='reference' if reference else 'linked',scenario=scenario,result='PASS'))
    return dict(result='PASS',checks=len(cases),cases=cases,boundary='Only GTA pool method, native GTA destructor and world removal are intercepted')

if __name__=='__main__':
    from verify_checkpoint32 import Gate
    import json
    print(json.dumps(exercise(Gate(sys.argv[1])),indent=2))
