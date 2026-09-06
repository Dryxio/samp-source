import json,sys,struct
from pathlib import Path
sys.path.insert(0,'tools')
from binary import COFF,PE,sha
from verify_checkpoint2 import import_slots
from verify_actor_closure import initial_bytes,Gate
old=json.load(open('config/checkpoint32/trial30-packets-contract.json'));new=json.load(open('build/actor-contract-draft.json'));prior={(r['unit'],r['rva'],r['size'],r['kind']):r for r in old['regions']}
run=Path('build/cp32-trial30-final-linked1');pe=PE('private/samp.dll');objs={p.stem:COFF(p) for p in run.glob('*.obj')}
reviews=[json.load(open(p)) for p in ['build/agent-independent/rakpeer-options-reviewed-manifest.json','build/agent-independent/network-simulator-reviewed-manifest.json','build/agent-independent/socket-bootstrap-reviewed-manifest.json','build/agent-textdraw/markers-reviewed-manifest.json']]
expected={(r['unit'],r['rva'],r['size']):r for v in reviews for r in v['regions']};seen=set();preserved=set()
for r in new['regions']:
 key=(r['unit'],r['rva'],r['size'],r['kind']);p=prior.get(key)
 if p:
  assert r['sha256']==p['sha256'];assert [(f['offset'],f['kind'],f['reference_va']) for f in r['fixups']]==[(f['offset'],f['kind'],f['reference_va']) for f in p['fixups']];r['implementation']=p['implementation'];preserved.add(key);continue
 key=key[:3];assert key in expected,('Unreviewed new region',r);seen.add(key);e=expected[key]
 o=objs[r['unit']];sec=o.sections[r['section']-1];assert sec['size']==r['size'] and r['offset']==0
 assert r['sha256']==e['sha256']==sha(initial_bytes(pe,r['rva'],r['size']))
 wanted=[(f['site_rva']-r['rva'],f['kind'],f['symbol'],f['target_va']) for f in e['bindings']]
 actual=[(f['offset'],f['kind'],f['symbol'],(f['reference_va']+struct.unpack_from('<I',sec['bytes'],f['offset'])[0])&0xffffffff) for f in r['fixups']]
 assert actual==wanted,(key,actual,wanted)
 r['implementation']='Normal C++ packet sync and complete BitStream templates, RakClient convenience; whole source regions and actual targets'
assert preserved==set(prior)
for key in expected:
 if key not in seen and expected[key]['kind']!='.CRT$XCU':assert any((r['rva'],r['size'])==key[1:] for r in old['regions']),('Missing reviewed section',key)
for n,e in new['externals'].items():
 if n in old['externals']:
  p=old['externals'][n];assert {k:v for k,v in p.items() if k not in ('import','library','chunks')}=={k:v for k,v in e.items() if k not in ('import','library','chunks')}
  for k in ('import','library','chunks'):
   if k in p:e[k]=p[k]
 else:
  wanted={'__imp__WSACleanup@0':(0x100e5464,('wsock32.dll','#116')),'__imp__gethostname@8':(0x100e5420,('wsock32.dll','#57'))}
  assert n in wanted,('Unreviewed external',n,e)
  va,identity=wanted[n];assert e['kind']=='import' and e['reference_va']==va and import_slots(pe)[va]==identity
  e['import']=list(identity)
assert set(new['externals'])-set(old['externals'])=={'__imp__WSACleanup@0','__imp__gethostname@8'}
for u,a in old['weak_aliases'].items():assert new['weak_aliases'][u]==a
for u,a in new['weak_aliases'].items():
 if u not in old['weak_aliases']:
  wanted={}
  for review in reviews:wanted.update(review.get('actual_weak_aliases',{}).get(u,{}))
  assert a==wanted,(u,a,wanted)
assert len(new['pending'])==3
for r in new['pending']:
 p=next(p for p in old['pending'] if p['anchor']==r['anchor']);r['reference_function_code_size']=p['reference_function_code_size']
 if 'limitation' in p:r['limitation']=p['limitation']
new['status']='REVIEWED';new['sdk']=json.load(open(run/'link.json',encoding='utf-8-sig'))['sdk'];assert new['sdk']==old['sdk']
new['review']=dict(old['review']);new['review'].update(scope='Final timed trial networking batch, exact unique gain determined by full frozenbase+latest union.',adaptations='Actual socket singleton and separate typed storage; original RakPeer options/simulator/connect. No invented allocation or maskedtargets.',reference_credit='New work duringtrial; direct0250.',verification='Whole original/linked COFF/PE and complete real dependencies; ten controls and Actor ABI.',excluded_candidates='Trailer sync mismatches excluded; no duplicate/CRT/data or unimplemented dependency credit.')
for name,file in [('itoa_qualification','build/trial30-final-itoa-qualified.json'),('linker_alias_provider_evidence','build/cp32-trial30-final-linked1-aliases.json'),('server_bridge_final','build/agent-independent/server-bridge-target-alias-qualification-cp32-trial30-final-linked1.json'),('logger_deleting_aliases','build/agent-independent/logger-deleting-aliases-cp32-trial30-final-linked1.json'),('network_tree_deleting_aliases','build/agent-independent/replica-network-id-aliases-qualified.json'),('sha1_deleting_alias_qualification','build/trial30-final-sha1-deleting-alias-qualified.json'),('mesh_deleting_alias_qualification','build/agent-independent/mesh-deleting-aliases-cp32-trial30-final-linked1.json'),('chat_navigation_alias','build/agent-textdraw/chat-navigation-linked-alias-review.json')]:new['review'][name]=json.load(open(file))
new['review']['rakclient_scope']=json.load(open('build/agent-independent/rakclient-messages-scope-reviewed.json'))
new['review']['router_scope']=json.load(open('build/agent-independent/router-processing-scope-reviewed.json'))
new['review']['raknet_command_alias']=json.load(open('build/agent-independent/raknet-command-parser-alias-qualified.json'))
new['review']['console_aliases']=json.load(open('build/agent-independent/console-aliases-qualified.json'))
new['review']['new_crt_wrapper_qualification']=json.load(open('build/rsa-commands-crt-qualified.json'))
new['review']['crt_locale_data_qualification']=json.load(open('build/crt-locale-data-qualified.json'))
new['review']['crt_locale_data_controls']=json.load(open('build/crt-locale-data-controls.json'))
new['review']['rpc_semantic_odr']=json.load(open('build/agent-textdraw/rpc-ped-vehicle-control-semantic-review.json'))
new['review']['rpc_duplicate_owner']=json.load(open('build/agent-textdraw/closure_rpc_ped_vehicle_control-linked-duplicates.json'))
new['review']['remote_sync']=json.load(open('build/agent-textdraw/remote-sync-complete-semantic-review.json'))
new['review']['rakserver_scope']=json.load(open('build/agent-independent/rakserver-messages-scope-reviewed.json'))
new['review']['packet_sync']=json.load(open('build/agent-textdraw/packet-sync-semantic-review.json'))
new['review']['rakclient_convenience']=json.load(open('build/agent-independent/rakclient-convenience-scope-reviewed.json'))
new['review']['socket_bootstrap']=json.load(open('build/agent-independent/socket-bootstrap-scope-reviewed.json'))
new['review']['network_simulator']=json.load(open('build/agent-independent/network-simulator-scope-reviewed.json'))
new['review']['markers']=json.load(open('build/agent-textdraw/markers-semantic-review.json'))
p=Path('config/checkpoint32/trial30-final-contract.json');p.write_text(json.dumps(new,indent=2)+'\n')
g=Gate(run.name,contract_path=str(p));g.bind();new['review']['associative_comdat_bindings']={'run':run.name,'bindings':g.associative_bindings,'negative_controls':json.load(open('build/associative-binding-controls.json')),'scope':'Complete strict associative matching, no duplicate credit.'};new['review']['crt_initializer_bindings']=g.crt_initializer_bindings;p.write_text(json.dumps(new,indent=2)+'\n');print('Reviewed',len(seen),'new whole sections, all',len(prior),'old preserved')

for key,e in expected.items():
 if e['kind']=='.CRT$XCU':
  rows=[b for b in g.crt_initializer_bindings if (b['unit'],b['original_rva'],b['size'])==key];assert len(rows)==1,(key,rows)
  b=rows[0];assert b['section']==e['section']
  assert [(x['offset'],x['provider']) for x in b['providers']]==[(x['site_rva']-e['rva'],x['symbol']) for x in e['bindings']]
print('Explicit reviewed CRT blocks matched through strict initializer binder')
