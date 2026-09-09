"""Review new runtime dependencies without granting them source coverage."""
import copy
from integration_pipeline import require,ROOT,read
from binary import PE,sha,u32
from verify_checkpoint2 import import_slots

def extend(base,extra):
    out=copy.deepcopy(base);pe=PE(ROOT/'private/samp.dll');inv={f['rva']:f for f in read(ROOT/'config/checkpoint31/inventory.json')['functions']}
    for name,e in extra.items():
        require(name not in base['externals'],'External already present')
        require(e['library'] in base['sdk'],'Unqualified library')
        rva=e['reference_va']-pe.base;raw=pe.read(rva,e['size']);require(sha(raw)==e['sha256'],'Changed external bytes')
        if e['kind']=='crt':
            f=inv[rva];require(f['name']==name and len(f['chunks'])==1 and f['chunks'][0]['size']==e['size'],'CRT identity or full extent mismatch')
            require(name=='__aullshr' and e['library']=='libcmt.lib','Only reviewed unsigned shift runtime supported')
            require(not any(rva<=a<rva+e['size'] for a in pe.relocations),'Unexpected CRT relocation')
        else:
            require(e['kind']=='import-thunk' and e['size']==6 and raw[:2]==b'\xff\x25','Invalid import thunk')
            require(u32(raw,2)==e['import_slot_va'],'IAT address differs')
            require(import_slots(pe)[u32(raw,2)]==tuple(e['import_identity']),'Import identity differs')
            require({a-rva for a in pe.relocations if rva<=a<rva+6}=={2},'Import relocations differ')
        out['externals'][name]=e
    return out
