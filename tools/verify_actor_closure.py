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
            elif r['kind']=='script-command':need(r['unit'] in ('closure_models','closure_camera','closure_audio') and r['size']==18 and not s['reloc_count'],'not a complete SCRIPT_COMMAND')
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

    def bind(self):
        for r in self.regions:
            if r['anchor'] in self.maps:
                self.locate(r,symbol_address(self.maps,r['anchor'])-r['anchor_offset'])
        progress=True
        while progress:
            progress=False
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
        else:
            need(e['kind']=='crt','unsupported external kind')
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
