"""Fail-closed gate for the frozen foundation regions and actual library linkage."""
import json
import re
from collections import Counter
from pathlib import Path
from capstone import CS_GRP_CALL,CS_GRP_JUMP
from capstone.x86 import X86_OP_IMM
from binary import COFF,PE,need,sha,u16,u32
from verify import ROOT,decode
from verify_checkpoint2 import map_symbols,symbol_address,import_slots


def read_json(path): return json.loads(Path(path).read_text(encoding='utf-8-sig'))


def archive_members(data):
    need(data.startswith(b'!<arch>\n'),'not a COFF archive')
    offset=8;longnames=b'';members={}
    while offset<len(data):
        head=data[offset:offset+60]
        need(len(head)==60 and head[58:]==b'`\n','bad archive header')
        size=int(head[48:58]);name=head[:16].decode('ascii').strip()
        start=offset+60;end=start+size
        need(end<=len(data),'truncated archive member')
        body=data[start:end]
        if name=='//': longnames=body
        elif name!='/':
            if name.startswith('/'):
                index=int(name[1:]);need(index<len(longnames),'bad long archive name')
                name=longnames[index:longnames.index(b'\0',index)].decode('ascii')
            else: name=name.rstrip('/')
            need(name not in members,'duplicate archive member')
            members[name]=body
        offset=end+(size&1)
    need(offset==len(data),'archive alignment mismatch')
    return members


def exports(pe):
    opt=u32(pe.data,0x3c)+24;rva=u32(pe.data,opt+96)
    table=pe.read(rva,40)
    count=u32(table,24);functions=u32(table,28);names=u32(table,32);ordinals=u32(table,36)
    result={}
    for i in range(count):
        at=u32(pe.read(names+i*4,4),0);name=bytearray()
        while pe.read(at,1)!=b'\0': name.extend(pe.read(at,1));at+=1
        ordinal=u16(pe.read(ordinals+i*2,2),0)
        result[name.decode('ascii')]=u32(pe.read(functions+ordinal*4,4),0)
    return result


def verify_region(contract,obj,reference,linked,address):
    raw,fixups=obj.function(contract['symbol'])
    need(not fixups,'RAW contract has a COFF relocation')
    need(len(raw)==contract['size'],'whole COMDAT size differs')
    need(sha(reference.read(contract['rva'],contract['size']))==contract['sha256'],'reference contract changed')
    need(raw==reference.read(contract['rva'],len(raw)),'original full bytes differ')
    need(raw==linked.read(address-linked.base,len(raw)),'actual linked full bytes differ')
    for pe,rva in [(reference,contract['rva']),(linked,address-linked.base)]:
        need(not any(rva<=site<rva+len(raw) for site in pe.relocations),'RAW region contains a PE relocation')
    instructions=decode(raw,reference.base+contract['rva'])
    starts={i.address for i in instructions};indirect=[]
    for ins in instructions:
        if not (ins.group(CS_GRP_CALL) or ins.group(CS_GRP_JUMP)):continue
        if ins.operands[0].type==X86_OP_IMM:
            need(ins.operands[0].imm in starts,'uncontracted direct code dependency')
        else:
            indirect.append(dict(offset=ins.address-reference.base-contract['rva'],kind=ins.mnemonic))
    return dict(**contract,linked_rva=address-linked.base,status='RAW_LINKED_EXACT',
                indirect_transfers=indirect,dependency_scope='caller body only; dynamic targets not certified' if indirect else 'no outbound direct/indirect code transfers')


class Gate:
    def __init__(self,run,check_fresh=True):
        self.directory=ROOT/'build'/run
        self.contract=read_json(ROOT/'config/checkpoint31/regions.json')
        self.reference=PE(ROOT/'private/samp.dll',self.contract['target_sha256'])
        self.linked=PE(self.directory/'foundation.dll')
        self.build=read_json(self.directory/'build.json')
        self.link=read_json(self.directory/'link.json')
        self.maps=map_symbols(self.directory/'foundation.map')
        self.inventory=read_json(ROOT/'config/checkpoint31/inventory.json')
        self.hints={f['rva']:f for f in self.inventory['functions']}
        self.objects={u['object']:COFF(self.directory/u['object']) for u in self.build['units']}
        if check_fresh:self.fresh()

    def fresh(self):
        pin=read_json(ROOT/'config/checkpoint31/toolchain.json')
        need(self.build['toolchain']==pin['tools'] and self.build['options']==pin['options'],'toolchain/options differ')
        need(pin['tools'][:4]==read_json(ROOT/'config/toolchain.json')['tools'],'foundation toolchain differs from original pin')
        groups=read_json(ROOT/'config/checkpoint31/build.json')['groups']
        expected={(g['name'],unit) for g in groups for unit in g['units']}
        need({(u['group'],u['unit']) for u in self.build['units']}==expected,'missing or unexpected compilation units')
        need(len(self.build['units'])==len(expected),'duplicate build unit')
        vendor=read_json(ROOT/'config/checkpoint31/vendor.json')['files']
        need(self.build['sources']==vendor,'vendor inputs changed')
        for file,digest in {**vendor,**self.link['inputs']}.items():
            need(sha((ROOT/file).read_bytes())==digest,'stale source/tool/config '+file)
            need(sha((self.directory/file).read_bytes())==digest,'altered input snapshot '+file)
        for unit in self.build['units']:
            need(sha(self.objects[unit['object']].data)==unit['object_sha256'],'altered object '+unit['object'])
        for name,digest in self.link['artifacts'].items():
            need(sha((self.directory/name).read_bytes())==digest,'altered linked artifact '+name)
        for group in groups:
            members=archive_members((self.directory/(group['name']+'.lib')).read_bytes())
            expected_members={u['object'] for u in self.build['units'] if u['group']==group['name']}
            need(set(members)==expected_members,'archive member inventory differs')
            for name,data in members.items():need(data==self.objects[name].data,'archive member differs from verified object')
        need(self.link['native_exit']==0,'native tests failed')
        need(self.link['sdk']==read_json(ROOT/'config/checkpoint31/sdk.json'),'SDK/CRT libraries changed')
        lock=read_json(ROOT/'config/checkpoint31/private-inputs.json')
        need(self.link['runtimes']=={name:lock[name] for name in ('d3dx9_25.dll','bass.dll')},'runtime dependencies changed')

    def verify(self):
        need(self.inventory['reference_sha256']==sha(self.reference.data),'inventory reference mismatch')
        for function in self.hints.values():
            for chunk in function['chunks']:
                need(sha(self.reference.read(chunk['rva'],chunk['size']))==chunk['sha256'],'inventory bytes changed')
        actual_exports=exports(self.linked);results=[];occupied=set();original=set()
        for index,contract in enumerate(self.contract['regions']):
            hint=self.hints[contract['rva']]
            need(len(hint['chunks'])==1 and hint['chunks'][0]['size']==contract['size'],'contract truncates or extends inventory boundary')
            address=symbol_address(self.maps,contract['symbol'])
            need(actual_exports['Region%04d'%index]==address-self.linked.base,'export and MAP disagree')
            for start,seen in [(contract['rva'],original),(address,occupied)]:
                span=set(range(start,start+contract['size']))
                need(not span.intersection(seen),'overlapping accepted regions')
                seen.update(span)
            results.append(verify_region(contract,self.objects[contract['object']],self.reference,self.linked,address))
        log=(self.directory/'native.log').read_text(encoding='utf-8-sig')
        match=re.search(r'PASS foundation: (\d+) checks;',log)
        need(match is not None and 'FAIL' not in log,'missing native PASS')
        loaded=re.search(r'foundation loaded at ([0-9A-Fa-f]+)',log)
        need(loaded is not None and int(loaded[1],16)==self.linked.base,'native loaded base differs')
        return dict(result='PASS',regions=results,unique_code_bytes=len(original),
                    native_checks=int(match[1]),native_log=log,
                    groups=dict(Counter(r['object'].split('_')[0] for r in results)),
                    actual_imports=[dict(rva=va-self.linked.base,dll=dll,name=name) for va,(dll,name) in sorted(import_slots(self.linked).items())],
                    scope='only frozen complete caller bodies RAW_LINKED_EXACT; complete libraries and game integration are not byte-certified',
                    build=self.build,link=self.link)


if __name__=='__main__':
    import argparse
    parser=argparse.ArgumentParser();parser.add_argument('--run',default='cp31-baseline');args=parser.parse_args()
    result=Gate(args.run).verify()
    (ROOT/'build'/args.run/'verification.json').write_text(json.dumps(result,indent=2)+'\n')
    print('PASS',len(result['regions']),'regions',result['unique_code_bytes'],'bytes')
