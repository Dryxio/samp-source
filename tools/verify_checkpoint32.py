"""Full linked matching for the incremental actor batch. No masks or unresolved fixups."""
import json,struct,re
from pathlib import Path
from binary import COFF,PE,need,sha,u32
from verify import ROOT,decode
from verify_checkpoint2 import map_symbols,symbol_address
from verify_checkpoint31 import read_json,exports
from capstone import CS_GRP_CALL,CS_GRP_JUMP
from capstone.x86 import X86_OP_IMM


class Gate:
    def __init__(self,run,check_fresh=True):
        self.directory=ROOT/'build'/run
        self.contract=read_json(ROOT/'config/checkpoint32/regions.json')
        self.reference=PE(ROOT/'private/samp.dll',self.contract['target_sha256'])
        self.linked=PE(self.directory/'actor.dll')
        self.maps=map_symbols(self.directory/'actor.map')
        self.build=read_json(self.directory/'build.json')
        self.objects={u['object']:COFF(self.directory/u['object']) for u in self.build['units']}
        if check_fresh:self.fresh()

    def fresh(self):
        pin=read_json(ROOT/'config/checkpoint31/toolchain.json')
        need(self.build['toolchain']==pin['tools'][:4],'changed toolchain')
        need(self.build['options']==pin['options'],'changed compiler options')
        current={str(p.relative_to(ROOT)) for p in (ROOT/'client').rglob('*') if p.is_file() and p.name!='.DS_Store'}
        need(current=={p for p in self.build['sources'] if p.startswith('client/')},'source inventory changed')
        for p,h in self.build['sources'].items():
            need(sha((ROOT/p).read_bytes())==h,'stale input '+p)
            need(sha((self.directory/p).read_bytes())==h,'altered snapshot '+p)
        for u in self.build['units']:
            need(sha((self.directory/u['object']).read_bytes())==u['object_sha256'],'altered object')
        for p,h in self.build['artifacts'].items():need(sha((self.directory/p).read_bytes())==h,'altered artifact '+p)
        sdk=read_json(ROOT/'config/checkpoint31/sdk.json')
        for p,h in self.build['sdk'].items():need(sdk[p]==h,'changed SDK '+p)
        need(set(self.build['sdk'])=={'kernel32.lib','libcmt.lib'},'unexpected SDK set')
        lock=read_json(ROOT/'config/checkpoint31/vendor.json')['files']
        need({p:h for p,h in self.build['sources'].items() if p.startswith('vendor/')}==lock,'vendor snapshot differs')
        need(self.build['native_exit']==0,'native test failed')
        m=re.search(r'PASS (\d+) native checks at ([0-9A-Fa-f]+)',(self.directory/'native.log').read_text(encoding='utf-8-sig'))
        need(m is not None and int(m[1])>0,'native result absent')
        need(int(m[2],16)==self.linked.base,'native DLL loaded at unexpected base')
        self.native_checks=int(m[1])

    def verify(self):
        records=self.contract['regions']+self.contract['data']
        bindings={};result=[]
        inv=read_json(ROOT/'config/checkpoint31/inventory.json')
        ranges={(f['rva'],f['chunks'][0]['size']) for f in inv['functions'] if len(f['chunks'])==1}
        for r in records:
            need(r['symbol'] not in bindings,'duplicate identity')
            bindings[r['symbol']]=(r,symbol_address(self.maps,r['symbol']))
        occupied=set()
        for r in records:
            obj=self.objects[r['unit']+'.obj'];iscode=r in self.contract['regions']
            syms=[s for s in obj.names.get(r['symbol'],[]) if s['section']>0]
            need(len(syms)==1,'ambiguous definition');sym=syms[0]
            sec=obj.sections[sym['section']-1]
            need(sym['value']==0 and sec['size']==r['size'],'complete section boundary mismatch '+r['symbol'])
            raw,fixups=obj.function(r['symbol']) if iscode else (obj.defined_data(sym,r['size']),[])
            need(iscode or not sec['reloc_count'],'uncontracted data fixups')
            need(sha(self.reference.read(r['rva'],r['size']))==r['sha256'],'changed reference contract')
            need(not iscode or (r['rva'],r['size']) in ranges,'not a full reference function')
            address=bindings[r['symbol']][1]
            span=set(range(address,address+len(raw)));need(not span&occupied,'overlapping linked bodies');occupied|=span
            deps=[]
            for linked in (False,True):
                pe=self.linked if linked else self.reference
                base=address if linked else pe.base+r['rva'];resolved=bytearray(raw);abs_sites=set()
                for f in fixups:
                    name=f['symbol']['name'];need(name in bindings,'uncontracted dependency '+name)
                    target,target_va=bindings[name]
                    target_va=target_va if linked else pe.base+target['rva']
                    off=f['offset'];value=u32(raw,off)+target_va
                    if f['kind']==20:value-=base+off+4
                    else:abs_sites.add(base-pe.base+off)
                    struct.pack_into('<I',resolved,off,value&0xffffffff)
                    if not linked:deps.append(dict(offset=off,kind=f['kind'],symbol=name,rva=target['rva']))
                need(bytes(resolved)==pe.read(base-pe.base,len(raw)),'full bytes differ '+r['symbol'])
                need({s for s in pe.relocations if base-pe.base<=s<base-pe.base+len(raw)}==abs_sites,'PE fixup set differs')
            indirect=[]
            if iscode:
                insns=decode(self.reference.read(r['rva'],r['size']),self.reference.base+r['rva'])
                starts={i.address for i in insns};known={self.reference.base+x['rva'] for x in self.contract['regions']}
                for ins in insns:
                    if not(ins.group(CS_GRP_CALL) or ins.group(CS_GRP_JUMP)):continue
                    if ins.operands[0].type==X86_OP_IMM:
                        need(ins.operands[0].imm in starts|known,'outbound uncontracted direct transfer')
                    else:indirect.append(dict(offset=ins.address-self.reference.base-r['rva'],instruction=ins.mnemonic))
            result.append(dict(**r,linked_rva=address-self.linked.base,
                               status='RESOLVED_LINKED_EXACT' if fixups else 'RAW_LINKED_EXACT',
                               dependencies=deps,indirect_transfers=indirect))
        need(set(exports(self.linked))=={r['symbol'] for r in self.contract['regions']},'unexpected exports')
        return dict(result='PASS',scope=self.contract['scope'],regions=result[:len(self.contract['regions'])],data=result[len(self.contract['regions']):],code_bytes=sum(r['size'] for r in self.contract['regions']))

if __name__=='__main__':
    import argparse
    p=argparse.ArgumentParser();p.add_argument('--run',default='cp32-baseline');a=p.parse_args()
    print(json.dumps(Gate(a.run).verify(),indent=2))
