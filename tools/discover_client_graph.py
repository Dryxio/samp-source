"""Discover full-section relocation proposals. Not an acceptance gate.

Derived target addresses require review before they can certify any caller.
Complete non-fixup bytes, section identity and symbol constraints must agree;
no proposal is reported as accepted or added to coverage.
"""
import argparse,json,struct
from collections import deque
from pathlib import Path
from binary import COFF,PE,need,sha,u32
from verify import ROOT
from verify_checkpoint2 import relocs


def discover(objects,seeds):
    ref=PE(ROOT/'private/samp.dll','b72b5dbe725f81864ca3f78bc7063bda56cc05fc7188af822fa7a754432553a2')
    inv=json.loads((ROOT/'config/checkpoint31/inventory.json').read_text())
    starts={f['rva']:f for f in inv['functions']}
    defs={}
    for unit,obj in objects.items():
        for name,syms in obj.names.items():
            for s in syms:
                if s['section']>0 and s['storage']==2:defs.setdefault(name,[]).append((unit,s))
    # MSVC emits weak vector-deleting aliases falling back to scalar deletion.
    for unit,obj in objects.items():
        d=obj.data;pointer=u32(d,8);count=u32(d,12);i=0
        while i<count:
            at=pointer+i*18;aux=d[at+17]
            if d[at+16]==105:
                need(aux==1 and u32(d,at+22) in (2,3),'unsupported weak external')
                alias=obj.symbols[i]['name'];fallback=obj.symbols[u32(d,at+18)]['name']
                if fallback in defs:defs.setdefault(alias,[]).extend(defs[fallback])
            i+=1+aux
    queue=deque();locations={};proposals=[];pending=[];constraints={}
    def push(unit,section,rva):
        key=(unit,section)
        if key in locations:
            if locations[key]!=rva:pending.append(dict(unit=unit,section=section,reason='inconsistent section placement',first_rva=locations[key],second_rva=rva))
            return
        locations[key]=rva;queue.append((unit,section,rva))
    for unit,name,rva in seeds:
        s=next(s for s in objects[unit].names[name] if s['section']>0)
        push(unit,s['section'],rva-s['value'])
    while queue:
        unit,idx,rva=queue.popleft();obj=objects[unit];sec=obj.sections[idx-1]
        names=[dict(name=s['name'],offset=s['value'],type=s['type']) for s in obj.symbols.values() if s['section']==idx and s['storage']!=3]
        row=dict(unit=unit,section=idx,rva=rva,size=sec['size'],section_name=sec['name'],symbols=names)
        if sec['uninitialized']:
            row['status']='ZERO_FILL_PLACEMENT_PROPOSAL';proposals.append(row);continue
        try:expected=ref.read(rva,sec['size'])
        except ValueError as e:pending.append(dict(**row,reason=str(e)));continue
        fixes=relocs(obj,idx);patched=bytearray(sec['bytes']);targets=[]
        for fix in fixes:
            off=fix['offset'];symbol=fix['symbol'];address=u32(expected,off)
            if fix['kind']==20:address=(address+ref.base+rva+off+4)&0xffffffff
            address=(address-u32(sec['bytes'],off))&0xffffffff
            value=address+u32(sec['bytes'],off)
            if fix['kind']==20:value-=ref.base+rva+off+4
            struct.pack_into('<I',patched,off,value&0xffffffff)
            targets.append(dict(offset=off,kind=fix['kind'],name=symbol['name'],address=address,defined_section=symbol['section'],symbol_offset=symbol['value']))
        if bytes(patched)!=expected:
            pending.append(dict(**row,reason='complete non-fixup bytes differ',difference_offsets=[i for i,(a,b) in enumerate(zip(patched,expected)) if a!=b][:16]));continue
        if sec['flags']&0x20 and rva in starts:
            f=starts[rva]
            if len(f['chunks'])!=1 or f['chunks'][0]['size']>sec['size']:
                pending.append(dict(**row,reason='reference function extends beyond source section'));continue
            row['code_size']=f['chunks'][0]['size']
        row.update(status='FULL_SECTION_PROPOSAL_NOT_ACCEPTED',sha256=sha(expected),targets=targets)
        proposals.append(row)
        for target in targets:
            name=target['name'];address=target['address'];key=(unit,name)
            need(key not in constraints or constraints[key]==address,'inconsistent target '+str(key));constraints[key]=address
            if target['defined_section']>0:
                push(unit,target['defined_section'],address-ref.base-target['symbol_offset'])
            elif name in defs:
                for dep,s in defs[name]:push(dep,s['section'],address-ref.base-s['value'])
            else:pending.append(dict(symbol=name,address=address,from_unit=unit,reason='unresolved external; not coverage'))
    return dict(status='PROPOSALS_ONLY_NOT_ACCEPTED',regions=proposals,pending=pending)

if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('spec');a=p.parse_args();spec=json.loads(Path(a.spec).read_text())
    objs={unit:COFF(ROOT/path) for unit,path in spec['objects'].items()}
    result=discover(objs,spec['seeds']);out=ROOT/'build/client-graph-proposals.json';out.write_text(json.dumps(result,indent=2)+'\n')
    print(len(result['regions']),'full section proposals;',len(result['pending']),'pending;',out)
