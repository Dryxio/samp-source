"""Draft the actor contract for review. Inferred addresses are never acceptance."""
import json,struct
from pathlib import Path
from collections import deque
from binary import COFF,PE,need,sha,u32
from verify import ROOT
from verify_checkpoint2 import relocs


def weak_aliases(obj):
    d=obj.data;offset=u32(d,8);count=u32(d,12);i=0;aliases={}
    while i<count:
        at=offset+i*18;aux=d[at+17]
        if d[at+16]==105:
            need(aux==1 and u32(d,at+22) in (2,3),'unknown weak external')
            aliases[obj.symbols[i]['name']]=obj.symbols[u32(d,at+18)]['name']
        i+=1+aux
    return aliases


def draft(run, extra_seeds=None):
    directory=ROOT/'build'/run
    objects={p.stem:COFF(p) for p in directory.glob('*.obj')}
    ref=PE(ROOT/'private/samp.dll','b72b5dbe725f81864ca3f78bc7063bda56cc05fc7188af822fa7a754432553a2')
    inv=json.loads((ROOT/'config/checkpoint31/inventory.json').read_text())
    hints={f['rva']:f for f in inv['functions']}
    definitions={};aliases={u:weak_aliases(o) for u,o in objects.items()}
    for unit,obj in objects.items():
        for name,syms in obj.names.items():
            for s in syms:
                if s['section']>0 and s['storage']==2:definitions.setdefault(name,[]).append((unit,s))
    queue=deque();locations={};regions=[];pending=[];externals={}
    def select(unit,symbol,rva):
        obj=objects[unit];index=symbol['section'];sec=obj.sections[index-1]
        offset=0;size=sec['size'];kind='code' if sec['flags']&0x20 else 'zero' if sec['uninitialized'] else 'data'
        if unit in ('closure_models','closure_camera','closure_audio','closure_vehicle','closure_player','game_menu','net_pickuppool') and kind=='data' and symbol['name'].startswith('_') and not symbol['name'].startswith('__'):
            offset=symbol['value'];size=18;kind='script-command'
        if unit=='closure_state':
            offset=symbol['value'];size=2 if symbol['name']=='?wVehicleComponentDebug@@3GA' else 4;kind='zero-object'
        base=rva-symbol['value']+offset;key=(unit,index,offset,size)
        if key in locations:
            need(locations[key]==base,'conflicting original placement '+str(key));return
        locations[key]=base;queue.append(dict(unit=unit,section=index,offset=offset,size=size,rva=base,kind=kind,anchor=symbol['name'],anchor_offset=symbol['value']-offset))
    explicit={
      'game_actorped':{'??0CEntity@@QAE@XZ':0x9c260,'??1CEntity@@UAE@XZ':0x9c280,'??_GCEntity@@UAEPAXI@Z':0x9c290,'??0CActorPed@@QAE@HMMMM@Z':0x9c2b0,'??1CActorPed@@UAE@XZ':0x9c750,'??_GCActorPed@@UAEPAXI@Z':0x9c7a0},
      'net_actorpool':{'??0CActorPool@@QAE@XZ':0x16c0,'?UpdateCount@CActorPool@@QAEXXZ':0x1660,'?Delete@CActorPool@@QAEHG@Z':0x16f0,'?FindIDFromGtaPtr@CActorPool@@QAEGH@Z':0x18b0,'?DeleteAll@CActorPool@@QAEXXZ':0x18e0},
      'game_scripting':{'?ScriptCommand@@YAHPBUSCRIPT_COMMAND@@ZZ':0xb2310,'?ExecuteScriptBuf@@YAHXZ':0xb22d0,'?InitScripting@@YAXXZ':0xb2550},
      'closure_chat':{'?Log@CChatWindow@@QAEXHPAD0@Z':0x677d0,'?AddDebugMessage@CChatWindow@@QAAXPADZZ':0x680f0,'?AddEntry@CChatWindow@@QAEXHPAD0KK@Z':0x67be0,'?PushBack@CChatWindow@@QAEXXZ':0x67450}}
    actor={'Destroy':0x9c400,'ApplyAnimation':0x9c460,'ClearAnimations':0x9c550,'SetTargetRotation':0x9c570,'GetHealth':0x9c5b0,'SetHealth':0x9c5d0,'GetArmour':0x9c610,'SetArmour':0x9c630,'GetStateFlags':0x9c650,'SetStateFlags':0x9c660,'IsDead':0x9c680,'GetActionTrigger':0x9c6b0,'SetActionTrigger':0x9c6c0,'IsInVehicle':0x9c6e0,'ToggleImmunity':0x9c700}
    for short,rva in actor.items():
        names=[n for n in objects['game_actorped'].names if n.startswith('?'+short+'@CActorPed@')]
        need(len(names)==1,'ambiguous actor member');explicit['game_actorped'][names[0]]=rva
    if extra_seeds:
        for unit,names in extra_seeds.items():explicit.setdefault(unit,{}).update(names)
    for unit,names in explicit.items():
        for name,rva in names.items():
            s=next(s for s in objects[unit].names[name] if s['section']>0);select(unit,s,rva)
    while queue:
        row=queue.popleft();unit=row['unit'];obj=objects[unit];sec=obj.sections[row['section']-1];off=row['offset'];size=row['size'];rva=row['rva']
        raw=bytes(size) if sec['uninitialized'] else sec['bytes'][off:off+size]
        fixes=relocs(obj,row['section']);need(all(off<=f['offset'] and f['offset']+4<=off+size for f in fixes) or not fixes,'partial section with fixups')
        if row['kind'].startswith('zero'):
            row.update(sha256=sha(raw),fixups=[]);regions.append(row);continue
        expected=ref.read(rva,size);patched=bytearray(raw);targets=[]
        for f in fixes:
            at=f['offset']-off;s=f['symbol'];address=u32(expected,at)
            if f['kind']==20:address=(address+ref.base+rva+at+4)&0xffffffff
            address=(address-u32(raw,at))&0xffffffff
            value=address+u32(raw,at)
            if f['kind']==20:value-=ref.base+rva+at+4
            struct.pack_into('<I',patched,at,value&0xffffffff)
            targets.append(dict(offset=at,kind=f['kind'],symbol=s['name'],reference_va=address))
        if bytes(patched)!=expected:
            pending.append(dict(**row,reference_sha256=sha(expected),reason='FULL_BYTES_DIFFER',difference_offsets=[i for i,(a,b) in enumerate(zip(patched,expected)) if a!=b]));continue
        if row['kind']=='code':
            need(rva in hints,'code section start is not an original function')
            ranges=[]
            for f in inv['functions']:
                if rva<=f['rva']<rva+size:
                    need(len(f['chunks'])==1 and f['chunks'][0]['rva']+f['chunks'][0]['size']<=rva+size,'partial reference function')
                    ranges.append(dict(offset=f['rva']-rva,size=f['chunks'][0]['size']))
            row['code_ranges']=ranges
        row.update(sha256=sha(expected),fixups=targets);regions.append(row)
        for f,target in zip(fixes,targets):
            s=f['symbol'];name=s['name'];address=target['reference_va']
            if s['section']>0:select(unit,s,address-ref.base);continue
            canonical=aliases[unit].get(name,name)
            if canonical in definitions:
                for provider,symbol in definitions[canonical]:select(provider,symbol,address-ref.base)
                continue
            need(name not in externals or externals[name]['reference_va']==address,'conflicting external '+name)
            kind='absolute-fs' if name=='__except_list' else 'import' if name.startswith('__imp_') else 'crt'
            record=dict(kind=kind,reference_va=address)
            if kind=='crt':
                f=hints.get(address-ref.base);need(f is not None and len(f['chunks'])==1,'unknown CRT entry boundary '+name)
                record.update(size=f['chunks'][0]['size'],sha256=f['chunks'][0]['sha256'])
            externals[name]=record
    return dict(status='DRAFT_REQUIRES_REVIEW',reference_sha256=sha(ref.data),run=run,regions=regions,pending=pending,externals=externals,weak_aliases=aliases)

if __name__=='__main__':
    import argparse
    p=argparse.ArgumentParser();p.add_argument('--run',default='cp32-closure-linked2');p.add_argument('--seeds');a=p.parse_args()
    result=draft(a.run,json.loads(Path(a.seeds).read_text()) if a.seeds else None);path=ROOT/'build/actor-contract-draft.json';path.write_text(json.dumps(result,indent=2)+'\n');print(len(result['regions']),'regions,',len(result['pending']),'pending;',path)
