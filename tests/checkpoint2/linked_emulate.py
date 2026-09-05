"""Execute the real linked Gravity -> Unprotect chain; mock only Windows API."""
import struct
import sys
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[2]/'tools'))
from binary import need
from verify_checkpoint2 import symbol_address
from unicorn import Uc, UC_ARCH_X86, UC_MODE_32, UC_HOOK_CODE
from unicorn.x86_const import *


def exercise(checkpoint):
    pe=checkpoint.linked
    gravity=symbol_address(checkpoint.maps,'?SetGravity@Game@@QAEXM@Z')
    unprotect=symbol_address(checkpoint.maps,'_Unprotect')
    iat=symbol_address(checkpoint.maps,'__imp__VirtualProtect@16')
    stop,api,stack=0x70000000,0x70001000,0x60008000
    samples=(0,0x80000000,0x3c03126f,0x3f800000,0xbf800000,0x7f800000)
    for bits in samples:
        uc=Uc(UC_ARCH_X86,UC_MODE_32)
        uc.mem_map(pe.base,(pe.image_size+4095)&~4095)
        for section in pe.sections: uc.mem_write(pe.base+section['rva'],section['bytes'])
        uc.mem_map(0x860000,0x10000)
        uc.mem_map(0x60000000,0x10000)
        uc.mem_map(stop,0x2000)
        uc.mem_write(iat,struct.pack('<I',api))
        uc.mem_write(stack,struct.pack('<II',stop,bits))
        uc.reg_write(UC_X86_REG_ESP,stack)
        uc.reg_write(UC_X86_REG_ECX,0x60000100)
        preserved={UC_X86_REG_EBX:0x1234,UC_X86_REG_ESI:0x2345,
                   UC_X86_REG_EDI:0x3456,UC_X86_REG_EBP:0x4567}
        for reg,value in preserved.items(): uc.reg_write(reg,value)
        seen=set();calls=[]
        def hook(machine,address,size,user):
            seen.add(address)
            if address==api:
                sp=machine.reg_read(UC_X86_REG_ESP)
                ret,where,length,protect,old=struct.unpack('<IIIII',machine.mem_read(sp,20))
                need((where,length,protect)==(0x863984,4,0x40),'wrong WinAPI arguments')
                need(0x60000000<=old<0x60010000-4,'old protection pointer outside stack')
                machine.mem_write(old,struct.pack('<I',2))
                machine.reg_write(UC_X86_REG_EAX,1)
                machine.reg_write(UC_X86_REG_ESP,sp+20)
                machine.reg_write(UC_X86_REG_EIP,ret)
                calls.append(where)
            else:
                need(gravity<=address<gravity+27 or unprotect<=address<unprotect+25,
                     'execution escaped the two reconstructed functions')
        uc.hook_add(UC_HOOK_CODE,hook)
        uc.emu_start(gravity,stop,count=1000)
        need(uc.reg_read(UC_X86_REG_EIP)==stop,'did not return')
        need(uc.reg_read(UC_X86_REG_ESP)==stack+8,'thiscall stack mismatch')
        need(bytes(uc.mem_read(0x863984,4))==struct.pack('<I',bits),'gravity store mismatch')
        need(gravity in seen and unprotect in seen and len(calls)==1,'chain not exercised')
        for reg,value in preserved.items(): need(uc.reg_read(reg)==value,'callee-saved register changed')
    return dict(result='PASS',checks=len(samples),base=hex(pe.base),
                scope='linked candidate only; Gravity and Unprotect execute; only VirtualProtect is mocked')
