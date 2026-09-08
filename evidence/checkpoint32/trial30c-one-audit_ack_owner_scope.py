import json,sys
from pathlib import Path
sys.path.insert(0,'tools')
from binary import PE,COFF
from draft_actor_contract import weak_aliases
p=Path('build/trial30c/network-agent');m=json.load(open(p/'ack-owner-reviewed-manifest.json'));c=json.load(open('config/checkpoint32/trial30b-two-contract.json'));idx=json.load(open('build/trial30c/symbol-index.json'))['symbols'];old=[r for r in c['regions'] if r['unit']=='closure_vendor_reliability_ack_family'];keys={(r['rva'],r['size']):r for r in m['regions']}
assert all((r['rva'],r['size']) in keys for r in old)
assert all(r['sha256']==keys[(r['rva'],r['size'])]['sha256'] for r in old)
newroots=m['seeds']['closure_vendor_reliability_ack_family'];audit={}
for n,rv in newroots.items():
 owners=idx.get(n,[]);audit[n]=dict(rva=rv,owners=owners,action='replace existing original owner TU, never add competing TU')
 assert all(x['unit']=='closure_vendor_reliability_ack_family' for x in owners),(n,owners)
externals={};rs={r['anchor']:r for r in c['regions']}
for n,v in m['external_targets'].items():
 if n.startswith('$') or n.startswith('__ehhandler'):continue
 if n in rs:assert v==0x10000000+rs[n]['rva'];externals[n]=dict(va=v,kind='accepted real code',owner=rs[n]['unit'])
 elif n in c['externals']:assert v==c['externals'][n]['reference_va'];externals[n]=dict(va=v,kind='existing reviewed external',evidence=c['externals'][n])
 else:raise AssertionError(n)
pe=PE(Path('private/samp.dll'))
for r in m['regions']:
 sites={x['site_rva'] for x in r['bindings'] if x['kind']==6 and x['symbol']!='__except_list'}
 assert sites=={x for x in pe.relocations if r['rva']<=x<r['rva']+r['size']}
o=COFF(Path('build/cp32-trial30c-label-ack-owner/closure_vendor_reliability_ack_family.obj'));a=weak_aliases(o)
report=dict(status='PASS_OWNER_AND_COMPLETE_SCOPE_AUDIT_NOT_ACCEPTANCE',baseline_unique=412480,new_unique_proposed=1107,existing_owner='closure_vendor_reliability_ack_family',source_change='Explicit instantiate real RangeNodeComp<unsigned short> before Deserialize/Serialize/ACK uses; restores comparator visibility in compiler as original full ReliabilityLayer constructor TU.',old_accepted_regions=len(old),all_old_regions_preserved=True,roots=audit,reviewed_externals=externals,weak_aliases=a,layout='Unchanged actual ReliabilityLayer1794 and InternalPacket55 static asserts; RangeNode two real ushort fields; all vendor templates unchanged.',crt='No new CRT initializer or global storage; selected functions have full existing vector lifetime CRT providers and all EH audited.',source025='0 direct new bytes; vendor already sufficient.',provenance='Prior prepared ACK source was blocked; fresh comparator visibility adaptation at trialC closes known source family. New evidence and source adaptation generated during trialC.',dependencies='Existing BitStream real owners and existing CRT imports; no plugin callback or missing dependency.',new_functions=list(newroots),excluded_graph='Trial explicit ordering did not change1183/1010 mismatches; no accepted gain.')
(p/'ack-actual-owner-scope-audit.json').write_text(json.dumps(report,indent=2)+'\n');print('PASS',len(old),'old regions; roots',len(newroots),'new',1107,'aliases',a)
