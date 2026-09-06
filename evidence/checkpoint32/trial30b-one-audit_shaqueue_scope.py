import json,hashlib
from pathlib import Path
b=Path('build/agent-network-b');m=json.load(open(b/'shaqueue-reviewed-manifest.json'));c=json.load(open('config/checkpoint32/trial30-final-contract.json'));p=json.load(open('evidence/checkpoint32/trial30-final-acceptance.json'));by={r['anchor']:r for r in p['regions']};out={}
for n,v in m['external_targets'].items():
 if n in c['externals']:assert c['externals'][n]['reference_va']==v;out[n]=c['externals'][n]
 elif n in by:assert by[n]['rva']+0x10000000==v;out[n]={k:by[n][k] for k in ('unit','rva','size')}
 elif any(r['rva']+0x10000000<=v<r['rva']+0x10000000+r['size'] for r in m['regions']):out[n]={'whole_local_region_verified':True,'reference_va':v}
 else:raise Exception((n,hex(v)))
scope=dict(status='PASS_FULL_SHA1_AND_TIMESTAMP_QUEUE_SCOPE',source025_direct_bytes=0,new_unique_code=m['new_unique_code'],providers=out,scope=['Original GetSHA1 constructs complete vendor CSHA1 (constructor/vtable/destructor dependencies already accepted), resets, updates, finalizes then copies exactly20bytes. FullGetSHA1172 and21byteEH included.', 'CheckSHA1 original78 calls GetSHA1 and compares20bytes with early mismatch false; no changes to original semantics.', 'Timestamp queue is actual DataStructures::Queue<RakNetTimeNS> where RakNetTimeNS is signed64bit; original ReliabilityLayer::Update calls Push443C0 and Compress44550. True queue pointer/head/tail/allocation fields, sizeof element8 and actual new/deletearray semantics preserved.', 'Push214 and Compress173 selected by unique complete bodies and original Update calltargets. No producer Update coverage claimed.', 'All6 whole regions and21fixups match with exact PErelocation sets;658unique code. No newexternalprovider or storage.', 'No025 gain: vendor complete.'],snapshots=[dict(path=str(f),sha256=hashlib.sha256(f.read_bytes()).hexdigest()) for f in [Path('vendor/upstream/raknet/ReliabilityLayer.h'),Path('vendor/upstream/raknet/DS_Queue.h'),Path('vendor/upstream/raknet/SHA1.h')]])
(b/'shaqueue-scope-reviewed.json').write_text(json.dumps(scope,indent=2)+'\n')
print(out)
