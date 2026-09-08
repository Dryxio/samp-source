import sys,json,struct
from pathlib import Path
sys.path.insert(0,'tools')
from binary import PE,COFF,sha,u32
from verify_checkpoint2 import relocs
from verify_actor_closure import initial_bytes
from rank_uncovered_candidates import accepted_code
run=Path('build/cp32-trial30d-two-ob1b');unit='eval_vendor_rakclient_small_wrappers';o=COFF(run/(unit+'.obj'));p=PE('private/samp.dll');po=COFF(run/'eval_vendor_rakpeer_small_provider_leaves.obj');plan=[]
for method,client_rva,peer_rva,peer_size in [('UnregisterAsRemoteProcedureCall',0x34530,0x38650,3),('SetRouterInterface',0x348c0,0x38bd0,13),('RemoveRouterInterface',0x348d0,0x38be0,25),('AllowConnectionResponseIPMigration',0x34940,0x38960,13),('GetMTUSize',0x34930,0x388b0,7)]:
 for cls,r,z in [('RakClient',client_rva,5),('RakPeer',peer_rva,peer_size)]:
  owner=o if cls=='RakClient' else po; s=next(s for s in owner.symbols.values() if s['section']>0 and s['name'].startswith('?'+method+'@'+cls+'@@'));plan.append((s,r,z,owner))
prov={s['name']:p.base+r for s,r,z,owner in plan};rows=[];seeds=[]
for s,r,z,owner in plan:
 unit='eval_vendor_rakclient_small_wrappers' if owner is o else 'eval_vendor_rakpeer_small_provider_leaves';sec=owner.sections[s['section']-1];assert sec['size']==z,(s['name'],sec['size'],z);raw=bytearray(sec['bytes']);fs=relocs(owner,s['section']);bs=[]
 for f in fs:
  assert f['kind']==20;off=f['offset'];target=prov[f['symbol']['name']]+u32(raw,off);struct.pack_into('<I',raw,off,(target-p.base-r-off-4)&0xffffffff);bs.append(dict(symbol=f['symbol']['name'],site_rva=r+off,kind=20,target_va=target))
 ref=initial_bytes(p,r,z);assert bytes(raw)==ref,(s['name'],hex(r));assert not any(r<=a<r+z for a in p.relocations)
 rows.append(dict(unit=unit,section=s['section'],anchor=s['name'],anchor_offset=0,rva=r,size=z,kind='code',sha256=sha(ref),bindings=bs));seeds.append(dict(unit=unit,symbol=s['name'],rva=r,size=z))
covered=accepted_code('evidence/checkpoint32/trial30d-one-acceptance.json');new={a for r in rows for a in range(r['rva'],r['rva']+r['size'])}-covered;assert len(new)==32,len(new)
Path('build/trial30d/closed-wrappers-reviewed-manifest.json').write_text(json.dumps(dict(status='PASS_WHOLE_REAL_PROVIDERS_CLOSED',run=run.name,new_unique_code=len(new),regions=rows,object_sha256=sha((run/(unit+'.obj')).read_bytes()),source_sha256=sha((run/('client/saco/'+unit+'.cpp')).read_bytes()),scope='5 vendor client wrappers plus5 genuine vendor peer leaves.54 historicallyaccepted providerbytes receivezeroadditionalcredit; newwrapper25+MTUgetter7.'),indent=2));Path('build/trial30d/closed-wrappers-seeds.json').write_text(json.dumps(seeds,indent=2));print('PASS32 newunique, fullproviderclosure')
