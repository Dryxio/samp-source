"""Replay every COFF fixup against reviewed R5 identities and the actual link.

Code sections are indivisible. Selected data subregions are complete typed
objects, never shortened function bodies. Unmatched providers bind callers but
are explicitly excluded from coverage.
"""
import json,struct
from pathlib import Path
from capstone.x86 import X86_REG_FS
from binary import COFF,PE,need,sha,u32
from verify import ROOT,decode
from verify_checkpoint2 import relocs,map_symbols,symbol_address,import_slots
from verify_checkpoint31 import read_json
from draft_actor_contract import weak_aliases


def initial_bytes(pe,rva,size):
    for s in pe.sections:
        delta=rva-s['rva'];extent=max(s['virtual_size'],s['size'])
        if 0<=delta and delta+size<=extent:
            data=s['bytes'][delta:delta+size] if delta<s['size'] else b''
            return data+bytes(size-len(data))
    raise ValueError('unmapped data range')


class Gate:
    def __init__(self,run='cp32-closure-linked2',fresh=True,contract_path=None):
        self.directory=ROOT/'build'/run
        self.contract_path=ROOT/(contract_path or 'config/checkpoint32/closure-contract.json')
        self.contract=read_json(self.contract_path)
        need(self.contract['run']==run,'contract belongs to a different link')
        need(self.contract['status']=='REVIEWED','unreviewed contract')
        self.reference=PE(ROOT/'private/samp.dll',self.contract['reference_sha256'])
        self.linked=PE(self.directory/'closure.dll');self.maps=map_symbols(self.directory/'closure.map')
        self.link=read_json(self.directory/'link.json')
        self.objects={p.stem:COFF(p) for p in self.directory.glob('*.obj')}
        self.regions=[dict(r,accepted=True) for r in self.contract['regions']]+[dict(r,accepted=False) for r in self.contract['pending']]
        self.by_section={};self.definitions={};self.actual_sections={}
        for r in self.regions:
            o=self.objects[r['unit']];s=o.sections[r['section']-1]
            need(r['offset']+r['size']<=s['size'],'region exceeds source section')
            need((r['kind']=='code')==bool(s['flags']&0x20),'code/data classification differs')
            need(r['kind'].startswith('zero')==s['uninitialized'],'zero/data classification differs')
            if r['kind']=='code':need(r['offset']==0 and r['size']==s['size'],'truncated source code')
            elif r['kind']=='script-command':need((r['unit'] in ('closure_models','closure_camera','closure_audio','closure_vehicle','closure_player','game_menu','net_pickuppool') or (r['unit']=='closure_rpc_checkpoints' and r['anchor'] in ('_disable_marker', '_set_marker_color', '_create_radar_marker_without_sphere', '_show_on_radar', '_create_racing_checkpoint', '_destroy_racing_checkpoint')) or (r['unit']=='closure_rpc_spectator_vehicle' and r['anchor'] in ('_make_actor_leave_car', '_rpc_set_car_z_angle')) or (r['unit']=='closure_player_lifetime' and r['anchor'] in ('_set_actor_weapon_droppable', '_set_actor_can_be_decapitated', '_rpc_ped_disassociate_object', '_rpc_ped_destroy_object_with_fade', '_rpc_ped_carry_object')) or (r['unit']=='closure_rpc_player_motion_shop' and r['anchor'] in ('_set_player_drunk_visuals', '_handling_responsiveness')) or (r['unit']=='closure_vehicle_destructor' and r['anchor']=='_disable_marker') or (r['unit']=='closure_vehicle_lifetime_helpers' and r['anchor']=='_remove_actor_from_car_and_put_at') or (r['unit']=='closure_remote_ped_constructor' and r['anchor'] in ('_set_actor_immunities','_set_actor_can_be_decapitated','_set_actor_weapon_droppable','_set_actor_money')) or (r['unit']=='closure_rpc_map_icons' and r['anchor']=='_disable_marker') or (r['unit']=='closure_remote_player_destruction' and r['anchor']=='_disable_marker') or (r['unit']=='closure_rpc_game_text' and r['anchor']=='_text_clear_all') or (r['unit']=='closure_vehicle_appearance_interior' and r['anchor'] in ('_rpc_select_ped_interior','_rpc_link_ped_interior','_rpc_refresh_ped_streaming')) or (r['unit']=='closure_object_release' and r['anchor'] in ('_is_model_available', '_rpc_release_model')) or (r['unit']=='closure_rpc_player_effects' and r['anchor'] in ('_toggle_widescreen','_create_explosion_with_radius')) or (r['unit']=='closure_pickup_create' and r['anchor'] in ('_request_model','_load_requested_models','_is_model_available','_destroy_pickup','_create_pickup_r5')) or (r['unit']=='closure_pickup_weapon' and r['anchor'] in ('_request_model','_load_requested_models','_is_model_available','_create_pickup_with_ammo'))) and r['size']==18 and not s['reloc_count'],'not a complete SCRIPT_COMMAND')
            elif r['kind']=='zero-object':need(r['size'] in (2,4) and s['uninitialized'],'invalid scalar zero object')
            else:need(r['offset']==0 and r['size']==s['size'],'truncated non-typed data')
            anchors=[v for v in o.names.get(r['anchor'],[]) if v['section']==r['section'] and v['value']==r['offset']+r['anchor_offset']]
            need(len(anchors)==1,'changed source anchor')
            key=(r['unit'],r['section']);self.by_section.setdefault(key,[]).append(r)
        for unit,o in self.objects.items():
            for name,syms in o.names.items():
                for s in syms:
                    if s['section']>0 and s['storage']==2:self.definitions.setdefault(name,[]).append((unit,s))
        need({u:weak_aliases(o) for u,o in self.objects.items()}==self.contract['weak_aliases'],'weak aliases changed')
        if fresh:self.fresh()
        self.bind()

    def fresh(self):
        pin=read_json(ROOT/'config/checkpoint31/toolchain.json')
        expected={t['name']:t['sha256'] for t in pin['tools']}
        need(self.link['linker_sha256']==expected['link.exe'],'changed linker')
        for name,h in self.link['artifacts'].items():need(sha((self.directory/name).read_bytes())==h,'altered linked artifact '+name)
        units=set()
        for run in self.link['probes']:
            p=read_json(self.directory/(run+'.json'));snapshot=ROOT/'build'/run
            need(p['tools']=={k:expected[k] for k in ('cl.exe','c1xx.dll','c2.dll')},'changed compiler')
            options=list(pin['options']);options+=['/GF']
            if any(u['unit']=='closure_world' for u in p['units']):options[options.index('/Ob1')]='/Ob2'
            need(p['options']==options,'unexpected compile profile')
            headers={n:h for n,h in p['sources'].items() if n.endswith(('.h','.hpp','.inl'))}
            for u in p['units']:
                name=u['object'];need(name not in units,'duplicate input object');units.add(name)
                need(sha((self.directory/name).read_bytes())==u['sha256'],'wrong linked input object')
                # Only sources actually consumed by this unit invalidate its cached object.
                inputs=dict(headers);cpp='client/saco/'+u['unit']+'.cpp';inputs[cpp]=p['sources'][cpp]
                for file,h in inputs.items():
                    need(sha((ROOT/file).read_bytes())==h,'stale compilation input '+file)
                    need(sha((snapshot/file).read_bytes())==h,'altered source snapshot '+file)
        need(units=={u+'.obj' for u in self.objects},'object set differs')
        sdk=read_json(ROOT/'config/checkpoint31/sdk.json')
        for name,h in self.link['sdk'].items():
            need(h==self.contract['sdk'][name],'SDK differs from reviewed link')
            if name in sdk:need(h==sdk[name],'SDK differs from foundation')

    def component(self,unit,symbol):
        parts=self.by_section.get((unit,symbol['section']),[])
        hits=[r for r in parts if r['offset']<=symbol['value']<r['offset']+r['size']]
        need(len(hits)==1,'uncontracted source dependency '+unit+':'+symbol['name'])
        return hits[0],symbol['value']-hits[0]['offset']

    def provider(self,unit,symbol):
        if symbol['section']>0:return self.component(unit,symbol)
        name=self.contract['weak_aliases'][unit].get(symbol['name'],symbol['name'])
        options=self.definitions.get(name,[])
        if options:
            targets=[self.component(u,s) for u,s in options]
            need(len({(r['rva']+offset) for r,offset in targets})==1,'ambiguous original source identity')
            return targets[0]
        need(name in self.contract['externals'],'unknown external '+name)
        return name,0

    def locate(self,r,va):
        key=(r['unit'],r['section']);base=va-r['offset']
        need(key not in self.actual_sections or self.actual_sections[key]==base,'inconsistent actual source section')
        self.actual_sections[key]=base
        for part in self.by_section[key]:part['linked_va']=base+part['offset']

    def anchor_address(self,r):
        symbols=self.objects[r['unit']].names.get(r['anchor'],[])
        local=any(s['section']==r['section'] and s['storage']==3 for s in symbols)
        if local:
            owner=(r['unit']+'.obj').lower()
            hits=[va for va,provider in self.maps[r['anchor']] if provider.lower()==owner]
            need(len(hits)==1,'missing/ambiguous object-local MAP symbol: '+r['unit']+':'+r['anchor'])
            return hits[0]
        return symbol_address(self.maps,r['anchor'])

    def discarded_associative_owner(self,r):
        # An inline COMDAT can be selected from another source object. Its
        # associative EH sections then have no independent MAP entry here.
        # Defer only that COFF-proven case; actual relocations must still bind
        # every byte to the selected parent's linked EH and unwind objects.
        obj=self.objects[r['unit']];sec=obj.sections[r['section']-1]
        if not sec['flags']&0x1000:return None
        local=any(x['section']==r['section'] and x['storage']==3 for x in obj.names.get(r['anchor'],[]))
        if not local or any(owner.lower()==r['unit'].lower()+'.obj' for _,owner in self.maps.get(r['anchor'],[])):return None
        ptr=u32(obj.data,8);associations=[]
        for index,symbol in obj.symbols.items():
            if symbol['section']!=r['section'] or symbol['storage']!=3 or symbol['type']!=0 or symbol['name']!=sec['name']:continue
            at=ptr+index*18
            if obj.data[at+17]!=1:continue
            aux=obj.data[at+18:at+36]
            if aux[14]==5:associations.append(struct.unpack_from('<H',aux,12)[0])
        if len(associations)!=1:return None
        parent=associations[0]
        if not 1<=parent<=len(obj.sections) or not obj.sections[parent-1]['flags']&0x1000:return None
        funcs=[x for x in obj.symbols.values() if x['section']==parent and x['storage']==2 and x['type']==0x20]
        if len(funcs)!=1:return None
        entries=self.maps.get(funcs[0]['name'],[])
        if len(entries)!=1:return None
        owner=entries[0][1]
        unit=next((u for u in self.objects if u.lower()+'.obj'==owner.lower()),None)
        if unit is None or unit==r['unit']:return None
        source=self.by_section.get((r['unit'],parent),[])
        selected=[q for q in self.regions if q['unit']==unit and q['anchor']==funcs[0]['name']]
        if len(source)!=1 or len(selected)!=1:return None
        a,b=source[0],selected[0]
        if not a['accepted'] or not b['accepted'] or a['kind']!='code' or (a['rva'],a['size'],a['sha256'])!=(b['rva'],b['size'],b['sha256']):return None
        return dict(unit=r['unit'],anchor=r['anchor'],section=r['section'],parent=funcs[0]['name'],selected_owner=owner,parent_linked_va=entries[0][0])

    def bind_crt_initializer(self,r):
        """Locate one compiler initializer slot inside the pinned CRT array only."""
        obj=self.objects[r['unit']];sec=obj.sections[r['section']-1]
        if sec['name']!='.CRT$XCU':return False
        need(r['accepted'] and r['kind']=='data' and r['offset']==0 and r['size']==sec['size']==4 and not sec['uninitialized'],'invalid complete CRT initializer section')
        fixes=relocs(obj,r['section'])
        need(len(fixes)==1 and fixes[0]['offset']==0 and fixes[0]['kind']==6 and u32(sec['bytes'],0)==0,'invalid CRT initializer relocation')
        target,offset=self.provider(r['unit'],fixes[0]['symbol'])
        need(not isinstance(target,str) and target['unit']==r['unit'] and target['accepted'] and target['kind']=='code' and offset==0,'CRT initializer lacks a complete local provider')
        need(r['rva'] in self.reference.relocations and u32(self.reference.read(r['rva'],4),0)==self.reference.base+target['rva'],'wrong original CRT initializer target or relocation')
        if 'linked_va' not in target:return False
        bounds=[]
        for name in ('___xc_a','___xc_z'):
            hits=self.maps.get(name,[])
            need(len(hits)==1 and hits[0][1].upper()=='LIBCMT:CRT0INIT.OBJ','invalid pinned CRT sentinel owner')
            bounds.append(hits[0][0]-self.linked.base)
        begin,end=bounds
        need(begin%4==0 and end%4==0 and begin+4<end,'invalid CRT initializer bounds')
        containers=[x for x in self.linked.sections if x['rva']<=begin and end+4<=x['rva']+x['size']]
        need(len(containers)==1 and self.linked.read(begin,4)==bytes(4) and self.linked.read(end,4)==bytes(4),'invalid CRT sentinel section or contents')
        hits=[a for a in range(begin+4,end,4) if u32(self.linked.read(a,4),0)==target['linked_va']]
        need(len(hits)==1,'missing or ambiguous CRT initializer slot')
        need(hits[0] in self.linked.relocations,'missing linked CRT initializer PE relocation')
        self.locate(r,self.linked.base+hits[0])
        proof=dict(unit=r['unit'],section=r['section'],original_rva=r['rva'],linked_rva=hits[0],size=4,provider=target['anchor'],provider_linked_va=target['linked_va'],sentinels=bounds,owner='LIBCMT:crt0init.obj')
        if proof not in self.crt_initializer_bindings:self.crt_initializer_bindings.append(proof)
        return True

    def bind(self):
        self.associative_bindings=[]
        self.crt_initializer_bindings=[]
        for r in self.regions:
            if r['anchor'] in self.maps:
                association=self.discarded_associative_owner(r)
                if association:
                    self.associative_bindings.append(association)
                    continue
                self.locate(r,self.anchor_address(r)-r['anchor_offset'])
        progress=True
        while progress:
            progress=False
            for r in self.regions:
                was_bound='linked_va' in r
                if self.bind_crt_initializer(r) and not was_bound:progress=True
            for r in self.regions:
                if 'linked_va' not in r or not r['accepted']:continue
                obj=self.objects[r['unit']];sec=obj.sections[r['section']-1]
                if sec['uninitialized']:continue
                raw=sec['bytes'][r['offset']:r['offset']+r['size']]
                actual=self.linked.read(r['linked_va']-self.linked.base,r['size'])
                for f in relocs(obj,r['section']):
                    target,offset=self.provider(r['unit'],f['symbol'])
                    if isinstance(target,str):continue
                    at=f['offset']-r['offset'];address=u32(actual,at)-u32(raw,at)
                    if f['kind']==20:address+=r['linked_va']+at+4
                    found=(address-offset)&0xffffffff
                    if 'linked_va' in target:need(target['linked_va']==found,'actual symbol target inconsistent')
                    else:self.locate(target,found);progress=True
        need(all('linked_va' in r for r in self.regions),'unresolved linked region')

    def external(self,name,linked):
        e=self.contract['externals'][name];actual=symbol_address(self.maps,name)
        if e['kind']=='absolute-fs':need(e['reference_va']==actual==0,'FS pseudo-symbol changed')
        elif e['kind']=='import':
            expected=tuple(e['import'])
            need(import_slots(self.reference).get(e['reference_va'])==expected,'wrong R5 import')
            need(import_slots(self.linked).get(actual)==expected,'wrong linked import')
        elif e['kind']=='import-thunk':
            expected=tuple(e['import_identity'])
            need(e['size']==6,'invalid import thunk size')
            for pe,va in ((self.reference,e['reference_va']),(self.linked,actual)):
                rva=va-pe.base;raw=pe.read(rva,6)
                need(raw[:2]==b'\xff\x25','changed import thunk instruction')
                need({x-rva for x in pe.relocations if rva<=x<rva+6}=={2},'import thunk PE relocations differ')
                need(import_slots(pe).get(u32(raw,2))==expected,'wrong import thunk destination')
                if pe is self.reference:
                    need(sha(raw)==e['sha256'] and u32(raw,2)==e['import_slot_va'],'changed original import thunk')
            need(e['library'] in self.contract['sdk'],'unreviewed import library')
            owner=e['library'].removesuffix('.lib').upper()+':'
            need(any(owner in provider.upper() for _,provider in self.maps[name]),'import thunk has a non-vendor provider')
        else:
            need(e['kind']=='crt','unsupported external kind')
            if 'chunks' in e:
                if not hasattr(self,'crt_inventory'):
                    self.crt_inventory={f['rva']:f for f in read_json(ROOT/'config/checkpoint31/inventory.json')['functions']}
                original=self.crt_inventory.get(e['reference_va']-self.reference.base)
                need(original is not None and e['chunks']==original['chunks'],'incomplete CRT reference chunks')
                for chunk in e['chunks']:
                    need(sha(self.reference.read(chunk['rva'],chunk['size']))==chunk['sha256'],'changed CRT reference chunk')
            else:
                need(sha(self.reference.read(e['reference_va']-self.reference.base,e['size']))==e['sha256'],'changed CRT reference entry')
            need(any('LIBCMT:' in owner.upper() or 'LIBCPMT:' in owner.upper() for _,owner in self.maps[name]),'CRT symbol has a non-vendor provider')
        return actual if linked else e['reference_va']

    def verify(self):
        inventory=read_json(ROOT/'config/checkpoint31/inventory.json')
        chunks={(c['rva'],c['size']) for f in inventory['functions'] for c in f['chunks']}
        results=[];code=set()
        for r in self.regions:
            if not r['accepted']:continue
            obj=self.objects[r['unit']];sec=obj.sections[r['section']-1]
            raw=bytes(r['size']) if sec['uninitialized'] else sec['bytes'][r['offset']:r['offset']+r['size']]
            fixes=relocs(obj,r['section']);need(len(fixes)==len(r['fixups']),'fixup count differs')
            expected=initial_bytes(self.reference,r['rva'],r['size'])
            need(sha(expected)==r['sha256'],'reference region hash differs')
            for f,reviewed in zip(fixes,r['fixups']):
                need((f['offset']-r['offset'],f['kind'],f['symbol']['name'])==(reviewed['offset'],reviewed['kind'],reviewed['symbol']),'fixup identity differs')
            for linked in (False,True):
                pe=self.linked if linked else self.reference;base=r['linked_va'] if linked else pe.base+r['rva'];patched=bytearray(raw);abs_sites=set()
                for f,reviewed in zip(fixes,r['fixups']):
                    target,offset=self.provider(r['unit'],f['symbol'])
                    if isinstance(target,str):
                        address=self.external(target,linked);original=self.contract['externals'][target]['reference_va']
                        fs=self.contract['externals'][target]['kind']=='absolute-fs'
                    else:
                        address=(target['linked_va'] if linked else pe.base+target['rva'])+offset
                        original=self.reference.base+target['rva']+offset;fs=False
                    need(original==reviewed['reference_va'],'reviewed target identity changed')
                    at=f['offset']-r['offset'];value=u32(raw,at)+address
                    if f['kind']==20:value-=base+at+4
                    elif not fs:abs_sites.add(base-pe.base+at)
                    if fs:
                        instructions=[ins for c in r.get("code_ranges",[]) for ins in decode(raw[c["offset"]:c["offset"]+c["size"]],base+c["offset"])]
                        containing=next(i for i in instructions if i.address-base<=at<i.address-base+i.size)
                        need(any(op.type==3 and op.mem.segment==X86_REG_FS for op in containing.operands),'FS fixup is not an FS operand')
                    struct.pack_into('<I',patched,at,value&0xffffffff)
                need(bytes(patched)==initial_bytes(pe,base-pe.base,len(raw)),'full region differs '+r['unit']+':'+r['anchor'])
                need({x for x in pe.relocations if base-pe.base<=x<base-pe.base+len(raw)}==abs_sites,'PE relocation set differs')
            for c in r.get('code_ranges',[]):
                need((r['rva']+c['offset'],c['size']) in chunks,'not a complete reference function range')
                decode(expected[c['offset']:c['offset']+c['size']],self.reference.base+r['rva']+c['offset'])
                code.update(range(r['rva']+c['offset'],r['rva']+c['offset']+c['size']))
            results.append(dict(unit=r['unit'],anchor=r['anchor'],rva=r['rva'],size=r['size'],kind=r['kind'],code_ranges=r.get('code_ranges',[]),linked_rva=r['linked_va']-self.linked.base,status='RESOLVED_LINKED_EXACT'))
        return dict(result='PASS',regions=results,code_bytes=len(code),pending=self.contract['pending'],scope='Only listed complete code/data regions; unmatched providers and CRT implementations excluded from coverage')

if __name__=='__main__':
    import argparse
    p=argparse.ArgumentParser();p.add_argument('--run',default='cp32-closure-linked2');p.add_argument('--contract');a=p.parse_args()
    result=Gate(a.run,contract_path=a.contract).verify();out=ROOT/'build'/a.run/'strict-verification.json';out.write_text(json.dumps(result,indent=2)+'\n');print(result['result'],len(result['regions']),'regions,',result['code_bytes'],'code bytes')
