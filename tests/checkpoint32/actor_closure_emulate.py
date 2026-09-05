"""Exercise construction and virtual pool deletion through real R5/linked bodies.
Only GTA operations, the Windows Sleep loader slot and CRT allocation/free are intercepted.
"""
import struct,sys
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[2]/'tools'))
from binary import need
from verify_checkpoint2 import symbol_address
from unicorn import Uc,UC_ARCH_X86,UC_MODE_32,UC_HOOK_CODE
from unicorn.x86_const import *

POOL,ACTOR,PED,VTABLE,GST,GAME,NATIVE_POOL,STACK=0x60000000,0x60006000,0x60010000,0x60011000,0x60012000,0x60013000,0x60014000,0x6003f000
STOP,SLEEP,PED_DELETE=0x70000000,0x70001000,0x70002000
SAVED={UC_X86_REG_EBX:0x12345678,UC_X86_REG_ESI:0x23456789,UC_X86_REG_EDI:0x3456789a,UC_X86_REG_EBP:0x456789ab}


def exercise(gate):
    reports=[]
    spawn="?New@CActorPool@@QAEHPAUACTOR_SPAWN_INFO@@@Z"
    use_entity_bridges=any(r["anchor"]=="?ApplyForce@CEntity@@QAEXMMMMMM@Z" and r["accepted"] for r in gate.regions)
    use_pool_new=any(r["anchor"]==spawn and r["accepted"] for r in gate.regions)
    for original in (True,False):
        pe=gate.reference if original else gate.linked
        def address(name):
            if not original:return symbol_address(gate.maps,name)
            for r in gate.regions:
                obj=gate.objects[r['unit']]
                for s in obj.names.get(name,[]):
                    if s['section']==r['section'] and r['offset']<=s['value']<r['offset']+r['size']:
                        return pe.base+r['rva']+s['value']-r['offset']
            return gate.contract['externals'][name]['reference_va']
        uc=Uc(UC_ARCH_X86,UC_MODE_32)
        uc.mem_map(pe.base,(pe.image_size+4095)&~4095)
        for s in pe.sections:uc.mem_write(pe.base+s['rva'],s['bytes'])
        for base,size in [(0,0x1000),(0x400000,0x5000),(0x469000,0x1000),(0xb74000,0x1000),(0x60000000,0x40000),(STOP,0x3000),(0x542000,0x1000),(0x4f6000,0x1000)]:uc.mem_map(base,size)
        def put(where,*values):uc.mem_write(where,struct.pack('<'+'I'*len(values),*values))
        def word(where):return struct.unpack('<I',uc.mem_read(where,4))[0]
        put(0,0xffffffff);put(0xb74490,NATIVE_POOL);put(PED,VTABLE);put(VTABLE,PED_DELETE)
        put(address('?gst@@3PAUGAME_SCRIPT_THREAD@@A'),GST)
        put(address('?pGame@@3PAVCGame@@A'),GAME)
        put(address('__imp__Sleep@4'),SLEEP)
        ranges=[(pe.base+r['rva']+c['offset'] if original else r['linked_va']+c['offset'],c['size']) for r in gate.regions if r['accepted'] for c in r.get('code_ranges',[])]
        trace=[];bridge_calls=[];allocations=[];model_queries=0;freed=[];ped_deleted=[];sleep_calls=[];lookups=[]
        def hook(machine,at,size,user):
            nonlocal model_queries
            sp=machine.reg_read(UC_X86_REG_ESP)
            def ret(cleanup,eax=None):
                if eax is not None:machine.reg_write(UC_X86_REG_EAX,eax)
                target=word(sp);machine.reg_write(UC_X86_REG_ESP,sp+4+cleanup);machine.reg_write(UC_X86_REG_EIP,target)
            if at==0x469eb0:
                need(machine.reg_read(UC_X86_REG_ECX)==GST,'wrong script this pointer')
                buf=address('?ScriptBuf@@3PAEA');need(word(GST+0x14)==buf,'script IP differs')
                op=struct.unpack('<H',uc.mem_read(buf,2))[0];trace.append(op)
                put(GST+0xc5,1)
                if op==0x248:
                    model_queries+=1;put(GST+0xc5,int(model_queries>=3))
                elif op==0x9a:
                    encoded=bytes(uc.mem_read(buf+2,28))
                    expected=b'\x01'+struct.pack('<i',5)+b'\x01'+struct.pack('<i',7)
                    expected+=b''.join(b'\x06'+struct.pack('<f',v) for v in (3.,4.,4.))+b'\x03\x00\x00'
                    need(encoded==expected,'create_actor varargs/float encoding differs');put(GST+0x3c,42)
                elif op==0x2ab:
                    expected=b''.join(b'\x01'+struct.pack('<i',v) for v in (42,1,1,1,1,1))
                    need(bytes(uc.mem_read(buf+2,len(expected)))==expected,'immunity script arguments differ')
                elif op not in (0x247,0x38b,0x173,0x446,0x60b):raise ValueError('unexpected GTA script opcode')
                ret(0);return
            if use_entity_bridges and at==0x542a50:
                need(machine.reg_read(UC_X86_REG_ECX)==PED,'wrong force this pointer')
                need(bytes(uc.mem_read(sp+4,24))==struct.pack('<6f',1.,2.,3.,4.,5.,6.),'wrong force arguments/order')
                bridge_calls.append('ApplyForce');ret(24);return
            if use_entity_bridges and at==0x4f6420:
                need(machine.reg_read(UC_X86_REG_ECX)==PED+0x138,'wrong audio subobject')
                need(word(sp+4)==123 and word(sp+8)==0,'wrong audio arguments')
                bridge_calls.append('PlayAudio');ret(8);return
            if at==SLEEP:
                need(word(sp+4)==1,'wrong Sleep argument');sleep_calls.append(1);ret(4);return
            if at==0x404910:
                need(machine.reg_read(UC_X86_REG_ECX)==NATIVE_POOL and word(sp+4)==42,'wrong GTA pool ABI')
                lookups.append(42);ret(4,PED);return
            if at==PED_DELETE:
                need(machine.reg_read(UC_X86_REG_ECX)==PED and word(sp+4)==1,'wrong native deletion ABI')
                ped_deleted.append(PED);ret(4);return
            if use_pool_new and at==address('??2@YAPAXI@Z'):
                need(word(sp+4)==0x56,'wrong actor allocation size')
                allocations.append(0x56);ret(0,ACTOR);return
            if at==address('??3@YAXPAX@Z'):
                need(word(sp+4)==ACTOR,'wrong CRT free pointer')
                need(word(ACTOR+0x40)==0 and word(ACTOR+0x48)==0,'actor not destroyed before free')
                freed.append(ACTOR);ret(0);return
            need(any(begin<=at<begin+length for begin,length in ranges),'execution escaped accepted source bodies at '+hex(at))
        uc.hook_add(UC_HOOK_CODE,hook);uc.reg_write(UC_X86_REG_FPCW,0x37f)
        def call(name,this,args):
            put(STACK,STOP,*args);uc.reg_write(UC_X86_REG_ESP,STACK);uc.reg_write(UC_X86_REG_ECX,this)
            for reg,value in SAVED.items():uc.reg_write(reg,value)
            uc.emu_start(address(name),STOP,count=100000)
            need(uc.reg_read(UC_X86_REG_EIP)==STOP,'did not return')
            need(uc.reg_read(UC_X86_REG_ESP)==STACK+4+4*len(args),'thiscall stack not restored')
            need(word(0)==0xffffffff,'SEH chain not restored')
            for reg,value in SAVED.items():need(uc.reg_read(reg)==value,'callee-saved register differs')
        call('??0CActorPool@@QAE@XZ',POOL,[])
        floats=[struct.unpack('<I',struct.pack('<f',v))[0] for v in (3.,4.,5.,90.)]
        if use_pool_new:
            payload=0x60015000
            uc.mem_write(payload,struct.pack('<HifffffB',5,7,3.,4.,5.,90.,75.,1))
            call(spawn,POOL,[payload])
            need(uc.reg_read(UC_X86_REG_EAX)==1 and allocations==[0x56],'pool creation failed')
            need(word(POOL)==5 and word(POOL+4+20)==ACTOR and word(POOL+0xfa4+20)==1 and word(POOL+0x1f44+20)==PED,'pool publication differs')
            need(word(POOL+0x2ee4+20)==1 and word(POOL+0x3e84+20)==0,'spawn flags differ')
            need(bytes(uc.mem_read(PED+0x540,4))==struct.pack('<f',75.) and bytes(uc.mem_read(ACTOR+0x55,1))==b'\x01','health/immunity fields differ')
        else:call('??0CActorPed@@QAE@HMMMM@Z',ACTOR,[7]+floats)
        need(word(ACTOR+0x44)==42 and word(ACTOR+0x48)==PED and word(ACTOR+0x40)==PED,'constructor state differs')
        need(word(ACTOR)==address('??_7CActorPed@@6B@'),'actor vtable differs')
        if not use_pool_new:put(POOL+4+5*4,ACTOR);put(POOL+0xfa4+5*4,1);put(POOL+0x1f44+5*4,PED)
        if use_entity_bridges:
            args=[struct.unpack('<I',struct.pack('<f',v))[0] for v in (1.,2.,3.,4.,5.,6.)]
            call('?ApplyForce@CEntity@@QAEXMMMMMM@Z',ACTOR,args)
            call('?PlayAudio@CEntity@@QAEXH@Z',ACTOR,[123])
            need(bridge_calls==['ApplyForce','PlayAudio'],'native bridges bypassed')
        call('?Delete@CActorPool@@QAEHG@Z',POOL,[5])
        need(uc.reg_read(UC_X86_REG_EAX)==1,'pool deletion failed')
        need(all(word(POOL+off+5*4)==0 for off in (4,0xfa4,0x1f44)),'slot not cleared')
        need(word(POOL)==0,'pool count not recomputed')
        need(word(ACTOR)==address('??_7CEntity@@6B@'),'base destruction vtable not restored')
        need(freed==[ACTOR] and ped_deleted==[PED] and len(lookups)==2,'deletion chain bypassed or duplicated')
        need(trace==[0x248,0x247,0x38b,0x248,0x248,0x9a,0x173,0x446,0x60b]+([0x2ab] if use_pool_new else []) and sleep_calls==[1],'model/script path differs')
        reports.append(dict(image='R5' if original else 'linked',result='PASS',pool_new=use_pool_new,native_bridges=bridge_calls,allocations=allocations,script_opcodes=trace,gta_lookups=len(lookups),native_deletions=len(ped_deleted),crt_frees=len(freed)))
    return dict(result='PASS',runs=reports,scope='Pool New and force/audio bridges when present, ctor, script encoding, polling, virtual deletion, SEH chain and nonvolatile registers; GTA/Windows/CRT boundary operations intercepted')

if __name__=='__main__':
    import json
    from verify_actor_closure import Gate
    gate=Gate();gate.verify();result=exercise(gate)
    (gate.directory/'closure-emulation.json').write_text(json.dumps(result,indent=2)+'\n');print(json.dumps(result,indent=2))
