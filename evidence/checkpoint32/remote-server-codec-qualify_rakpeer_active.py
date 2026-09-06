from pathlib import Path
import sys,json,hashlib
sys.path.insert(0,'tools')
from binary import PE,COFF
from draft_actor_contract import weak_aliases
b=Path('build/agent-independent'); run=Path('build/cp32-remote-server-codec-ob1-v2'); unit='eval_vendor_rakpeer_active';name='?IsActive@RakPeer@@UBE_NXZ';rva=0x3e040
c=COFF(run/(unit+'.obj'));p=PE(Path('private/samp.dll'))
s=next(s for s in c.names[name] if s['section']>0);sec=c.sections[s['section']-1]
assert s['value']==0 and sec['size']==11 and sec['reloc_count']==0
assert sec['bytes']==p.read(rva,11)
assert not [v for v in p.relocations if rva<=v<rva+11]
m=json.loads((b/'rakserver-messages-reviewed-manifest.json').read_text())
assert unit not in m['seeds']
m['seeds'][unit]={name:rva};m['roots'][name]=rva
m['regions'].append(dict(unit=unit,section=s['section'],rva=rva,size=11,kind=sec['name'],sha256=hashlib.sha256(sec['bytes']).hexdigest(),whole_bytes_equal=True,bindings=[]))
from rank_uncovered_candidates import accepted_code
covered=accepted_code('evidence/checkpoint32/client-router-rpc-acceptance.json')
extra=set(range(rva,rva+11))
m['new_unique_code']+=len(extra-covered);m['already_covered_code']+=len(extra&covered);m['physical_code_compared']+=11
m['source_files'].append(dict(path='client/saco/'+unit+'.cpp',sha256=hashlib.sha256(Path('client/saco/'+unit+'.cpp').read_bytes()).hexdigest()))
m['objects'][unit]=hashlib.sha256((run/(unit+'.obj')).read_bytes()).hexdigest();m['actual_weak_aliases'][unit]=weak_aliases(c)
m['external_targets'].pop(name,None)
m['identity_reasons'][name]='Original RakServer GetStaticServerData 43170 direct call to 3e040; full real RakPeer type endThreads predicate const ABI. No allocation or VFT credited.'
m['provider_closure_correction']='Previous preflight wrongly left IsActive as unimplemented external; now actual complete 11-byte source provider included.'
(b/'rakserver-messages-reviewed-manifest.json').write_text(json.dumps(m,indent=2)+'\n');(b/'rakserver-messages-seeds.json').write_text(json.dumps(m['seeds'],indent=2)+'\n')
print('PASS',m['new_unique_code'],len(m['regions']))
