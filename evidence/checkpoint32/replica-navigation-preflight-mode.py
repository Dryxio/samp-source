from pathlib import Path
import json,sys,struct
sys.path.insert(0,'tools');from binary import COFF,PE,sha,u32
run=Path('build/cp32-network-id-mode-trial1');u='closure_vendor_network_id_mode';name='?IsPeerToPeerMode@NetworkID@@SA_NXZ';o=COFF(run/(u+'.obj'));p=PE('private/samp.dll');raw,fx=o.function(name);assert len(raw)==6 and len(fx)==1
f=fx[0];assert f['kind']==6 and f['offset']==1 and f['symbol']['name']=='?peerToPeerMode@NetworkID@@2_NA';old=json.load(open('config/checkpoint32/connection-graph-contract.json'));r=next(r for r in old['regions'] if r['anchor']==f['symbol']['name']);assert r['unit']=='closure_vendor_network_id_comparisons' and r['rva']==0x118bc0
expected=bytearray(raw);struct.pack_into('<I',expected,1,p.base+r['rva']);assert bytes(expected)==p.read(0x339c0,6);assert {a for a in p.relocations if 0x339c0<=a<0x339c6}=={0x339c1};sym=next(s for s in o.names[name] if s['section']>0)
out=dict(status='PASS_TRUE_NETWORK_ID_MODE_GETTER',run=str(run),seeds={u:{name:0x339c0}},regions=[dict(unit=u,section=sym['section'],rva=0x339c0,size=6,kind='.text',sha256=sha(expected),whole_bytes_equal=True,bindings=[dict(symbol=f['symbol']['name'],site_rva=0x339c1,target_va=p.base+r['rva'],kind=6)])],new_unique_code=6,source025_direct=0,source_sha256=sha(Path('client/saco/'+u+'.cpp').read_bytes()),object_sha256=sha((run/(u+'.obj')).read_bytes()))
Path('build/network-id-mode-reviewed.json').write_text(json.dumps(out,indent=2)+'\n');print('PASS getter6, actual existing flag owner')
