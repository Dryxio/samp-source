"""Isolated x86 ABI/behavior checks; never loads a candidate into GTA.

Both reference and reconstructed bodies run at two bases. External routines
Delete and Unprotect are intercepted mocks, explicitly outside test coverage.
"""
import argparse
import json
import math
import struct
import sys
from pathlib import Path

from unicorn import Uc, UC_ARCH_X86, UC_MODE_32, UC_HOOK_CODE
from unicorn.x86_const import *

sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from binary import need
from verify import ROOT, load, relocate

OBJECT, OTHER, STACK, STOP, OUTPUT = 0x50000000,0x50010000,0x6000f000,0x70000000,0x70001000


def f32(value):
    return struct.unpack('<f',struct.pack('<f',value))[0]


class Capsule:
    def __init__(self, manifest, pe, objects, base, candidate):
        self.manifest,self.pe,self.base = manifest,pe,base
        self.fns = {f['name']:f for f in manifest['functions']}
        self.u = u = Uc(UC_ARCH_X86,UC_MODE_32)
        u.mem_map(base,(pe.image_size+0xfff)&~0xfff)
        for s in pe.sections:
            if s['size']:
                u.mem_write(base+s['rva'],s['bytes'])
        for rva in pe.relocations:
            self.put(base+rva,(self.get(base+rva)+base-pe.base)&0xffffffff)
        if candidate:
            for fn in manifest['functions']:
                code,_ = relocate(objects[fn['unit']],fn,manifest,pe,base)
                u.mem_write(base+fn['rva'],code)
        for start,size in [(OBJECT,0x20000),(0x60000000,0x10000),(STOP,0x2000),
                           (0x863000,0x1000),(0xc81000,0x1000),
                           (0xb6f000,0x1000),(0xc3e000,0x1000)]:
            u.mem_map(start,size)
        self.active = (0,0)
        self.calls = []
        self.capture = False
        u.hook_add(UC_HOOK_CODE,self.trace)

    def put(self,addr,value): self.u.mem_write(addr,struct.pack('<I',value&0xffffffff))
    def get(self,addr): return struct.unpack('<I',self.u.mem_read(addr,4))[0]
    def pf(self,addr,value): self.u.mem_write(addr,struct.pack('<f',value))
    def gf(self,addr): return struct.unpack('<f',self.u.mem_read(addr,4))[0]
    def symbol(self,name): return self.base+self.manifest['symbols'][name]['rva']

    def trace(self,u,address,size,_):
        if self.capture and STOP+0x20 <= address < STOP+0x26:
            return
        if self.active[0] <= address < self.active[1]:
            return
        sp = u.reg_read(UC_X86_REG_ESP)
        if address == self.symbol('?Delete@ActorPool@@QAEHG@Z'):
            self.calls.append(('Delete',u.reg_read(UC_X86_REG_ECX),self.get(sp+4)))
            clean = 8
        elif address == self.symbol('_Unprotect'):
            self.calls.append(('Unprotect',self.get(sp+4),self.get(sp+8)))
            clean = 4
        else:
            raise ValueError('unexpected execution outside selected body: %#x'%address)
        ret = self.get(sp)
        u.reg_write(UC_X86_REG_ESP,sp+clean)
        u.reg_write(UC_X86_REG_EAX,1)
        u.reg_write(UC_X86_REG_ECX,0xcccccccc)
        u.reg_write(UC_X86_REG_EDX,0xdddddddd)
        u.reg_write(UC_X86_REG_EIP,ret)

    def call(self,name,args=(),floating=False):
        fn = self.fns[name]
        self.active = (self.base+fn['rva'],self.base+fn['rva']+fn['size'])
        self.calls = []
        self.put(STACK,STOP)
        for i,value in enumerate(args): self.put(STACK+4+4*i,value)
        u = self.u
        saved = {UC_X86_REG_EBX:0x13579,UC_X86_REG_ESI:0x24680,
                 UC_X86_REG_EDI:0x98765,UC_X86_REG_EBP:0x54321}
        for reg,val in saved.items(): u.reg_write(reg,val)
        u.reg_write(UC_X86_REG_EAX,0xaabbccdd)
        u.reg_write(UC_X86_REG_ECX,OBJECT)
        u.reg_write(UC_X86_REG_EDX,0xeeeeeeee)
        u.reg_write(UC_X86_REG_ESP,STACK)
        u.reg_write(UC_X86_REG_EFLAGS,0x202)
        u.reg_write(UC_X86_REG_FPCW,0x37f)
        u.emu_start(self.active[0],STOP,count=250000)
        need(u.reg_read(UC_X86_REG_EIP)==STOP,'function did not return within instruction budget')
        need(u.reg_read(UC_X86_REG_ESP)==STACK+4+4*len(args),'wrong stdcall/thiscall stack cleanup')
        for reg,val in saved.items(): need(u.reg_read(reg)==val,'callee-saved register clobbered')
        value = u.reg_read(UC_X86_REG_EAX)
        if floating:
            # Harness-only FSTP [OUTPUT] serializes the x87 return value as float.
            # This trampoline is not reconstructed source or counted coverage.
            u.mem_write(STOP+0x20,b'\xd9\x1d'+struct.pack('<I',OUTPUT))
            self.capture = True
            u.emu_start(STOP+0x20,STOP+0x26,count=1)
            self.capture = False
            value = self.gf(OUTPUT)
        return value


def exercise(c):
    checks = []

    def check(name,actual,expected,floating=False):
        ok = math.isclose(actual,expected,rel_tol=1e-6,abs_tol=1e-6) if floating else actual==expected
        need(ok,f'{name}: {actual!r} != {expected!r}')
        checks.append(dict(case=name,result='PASS'))

    c.u.mem_write(OBJECT,bytes(0x10000))
    for index in [0,1,1003,1004,65535]:
        if index<1004: c.put(OBJECT+0x2a+4*index,0x12340000+index)
        check(f'slot-{index}',c.call('PlayerPool.GetSlotState',[index]),
              0x12340000+index if index<1004 else 0)
    for index in [0,1003,1004,1005,65535]:
        # R5's GetAt uses >1004, unlike GetSlotState's >=1004. Preserve it.
        if index<=1004: c.put(OBJECT+0x1f8a+index*4,OTHER)
        c.put(OTHER+0x10,0xface1234)
        check(f'get-at-{index}',c.call('PlayerPool.GetAt',[index]),0xface1234 if index<=1004 else 0)
    c.put(OBJECT+0x1f8a,0)
    check('get-at-null',c.call('PlayerPool.GetAt',[0]),0)
    c.u.mem_write(OBJECT+0x94,struct.pack('<HHH',3,2,1))
    for args in [(1,2,3),(0,2,3),(1,0,3),(1,2,0),(65535,2,3)]:
        check('keys-'+str(args),c.call('LocalPlayer.AreKeysChanged',args),int(args!=(1,2,3)))

    c.u.mem_write(OBJECT,b'\xa5'*12008)
    check('ctor-return-this',c.call('ActorPool.ctor'),OBJECT)
    check('ctor-zero-all-3001-dwords',bytes(c.u.mem_read(OBJECT,12004)),bytes(12004))
    check('ctor-no-overwrite-after',bytes(c.u.mem_read(OBJECT+12004,4)),b'\xa5'*4)
    for active in [[],[0],[1],[4],[5],[998],[999],[3,57,811]]:
        c.u.mem_write(OBJECT+0xfa4,bytes(4000))
        for index in active: c.put(OBJECT+0xfa4+4*index,1)
        c.call('ActorPool.UpdateCount')
        check('last-active-'+str(active),c.get(OBJECT),max(active) if active else 0)
    for end,index in [(-1,None),(0,None),(0,0),(999,999),(999,500)]:
        c.put(OBJECT,end)
        c.u.mem_write(OBJECT+0x1f44,bytes(4000))
        if index is not None: c.put(OBJECT+0x1f44+4*index,0x12345678)
        result = c.call('ActorPool.FindIDFromGtaPtr',[0x12345678])&0xffff
        check(f'find-{end}-{index}',result,index if index is not None else 65535)
    c.call('ActorPool.DeleteAll')
    check('delete-all-1000-thiscall-targets',c.calls,[('Delete',OBJECT,i) for i in range(1000)])

    for locked in [0,1,0xffffffff]:
        c.put(OBJECT+0x69,locked)
        for addr in [0xc81318,0xc8131c,0xc81320]: c.put(addr,0x5555)
        c.call('Game.SetWeather',[23])
        check(f'weather-current-{locked}',c.get(0xc81318),23)
        for addr in [0xc8131c,0xc81320]: check(f'weather-{addr:x}-{locked}',c.get(addr),23 if locked==0 else 0x5555)
    for value in [0.008,0.0,-1.0,1.0]:
        bits=struct.unpack('<I',struct.pack('<f',value))[0]
        c.call('Game.SetGravity',[bits])
        check('gravity-store-'+str(value),c.get(0x863984),bits)
        check('gravity-cdecl-args-'+str(value),c.calls,[('Unprotect',0x863984,4)])

    zoomptr,aspectptr=0xb6f250,0xc3efa4
    c.put(c.symbol('_cameraZoom'),zoomptr)
    c.put(c.symbol('_aspectRatio'),aspectptr)
    c.pf(zoomptr,52.5);c.pf(aspectptr,1.25)
    c.call('StoreLocalCamera')
    check('camera-store-zoom',c.gf(c.symbol('_localZoom')),52.5)
    check('camera-store-aspect',c.gf(c.symbol('_localAspect')),1.25)
    c.pf(zoomptr,0);c.pf(aspectptr,0)
    c.call('RestoreLocalCamera')
    check('camera-restore-zoom',c.gf(zoomptr),52.5)
    check('camera-restore-aspect',c.gf(aspectptr),1.25)
    for value in [0.0,1.0,1.25,2.5,-1.0]:
        c.pf(aspectptr,value)
        check('aspect-'+str(value),c.call('GetLocalAspect',floating=True),value-1.0,True)
    for value in [0.0,35.0,52.5,70.0,100.0,-35.0]:
        c.pf(zoomptr,value)
        check('zoom-'+str(value),c.call('GetLocalZoom',floating=True),(value-35.0)/35.0,True)
    for index,zoom,aspect in [(0,0.0,0.0),(1,0.5,0.25),(17,-0.25,-0.5),(209,1.0,1.5)]:
        args=[index,*struct.unpack('<II',struct.pack('<ff',zoom,aspect))]
        c.call('SetPlayerCamera',args)
        check(f'player-zoom-{index}',c.gf(c.symbol('_remoteZoom')+4*index),zoom)
        check(f'player-aspect-{index}',c.gf(c.symbol('_remoteAspect')+4*index),aspect)
        c.call('SetRemoteCamera',[index])
        check(f'remote-zoom-{index}',c.gf(zoomptr),zoom*35.0+35.0,True)
        check(f'remote-aspect-{index}',c.gf(aspectptr),aspect+1.0,True)
    return checks


def main():
    parser=argparse.ArgumentParser();parser.add_argument('--run',default='baseline')
    args=parser.parse_args()
    manifest,pe,objects,_=load(args.run)
    runs=[]
    for base in [0x10000000,0x30000000]:
        for candidate in [False,True]:
            capsule=Capsule(manifest,pe,objects,base,candidate)
            checks=exercise(capsule)
            name='compiled' if candidate else 'reference'
            print(f'PASS {name} at {base:#x}: {len(checks)} behavior checks; ABI checked on every call')
            runs.append(dict(base=base,variant=name,checks=checks))
    result=dict(result='PASS',environment='Unicorn x86 emulation, not native game runtime',
                mocks=['ActorPool.Delete','Unprotect'],runs=runs,
                checks=sum(len(r['checks']) for r in runs))
    (ROOT/'build'/args.run/'emulation.json').write_text(json.dumps(result,indent=2)+'\n')


if __name__=='__main__': main()
