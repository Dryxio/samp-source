import json,sys,struct
from pathlib import Path
sys.path.insert(0,'tools')
from binary import COFF,PE,sha
from verify_checkpoint2 import import_slots
from verify_actor_closure import initial_bytes,Gate
old=json.load(open('config/checkpoint32/replica-navigation-contract.json'));new=json.load(open('build/actor-contract-draft.json'));prior={(r['unit'],r['rva'],r['size'],r['kind']):r for r in old['regions']}
run=Path('build/cp32-tcp-font-linked1');pe=PE('private/samp.dll');objs={p.stem:COFF(p) for p in run.glob('*.obj')}
reviews=[json.load(open(p)) for p in ['build/agent-independent/tcp-reviewed-manifest.json','build/agent-textdraw/font-helpers-reviewed-manifest.json','build/font-stores-reviewed.json']]
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
 r['implementation']='Normal C++ complete TCPInterface/RakSleep or font helper with real static state; all original and linked targets'
assert preserved==set(prior)
for key in expected:
 if key not in seen:assert any((r['rva'],r['size'])==key[1:] for r in old['regions']),('Missing reviewed section',key)
imports={'__imp__htonl@4':('wsock32.dll','#8'),'__imp__listen@8':('wsock32.dll','#13'),'__imp__CloseHandle@4':('kernel32.dll','CloseHandle'),'__imp__send@16':('wsock32.dll','#19'),'__imp__select@20':('wsock32.dll','#18'),'__imp__accept@12':('wsock32.dll','#1'),'__imp__recv@16':('wsock32.dll','#16'),'__imp__closesocket@4':('wsock32.dll','#3'),'__imp__WSAStartup@8':('wsock32.dll','#115')}
for n,e in new['externals'].items():
 if n in old['externals']:
  p=old['externals'][n];assert {k:v for k,v in p.items() if k not in ('import','library','chunks')}=={k:v for k,v in e.items() if k not in ('import','library','chunks')}
  for k in ('import','library','chunks'):
   if k in p:e[k]=p[k]
 elif n in imports:
  assert e['kind']=='import' and import_slots(pe)[e['reference_va']]==imports[n];e['import']=list(imports[n])
 elif n=='___WSAFDIsSet@8':
  assert e['kind']=='import-thunk' and e['reference_va']==pe.base+0xc61e4 and e['import_identity']==['wsock32.dll','#151'];e['library']='wsock32.lib'
 elif n=='__beginthreadex':assert e['kind']=='crt' and e['reference_va']==pe.base+0xc8de8 and e['size']==139
 else:raise AssertionError(('Unexpected new external',n,e))
assert {n for n in new['externals'] if n not in old['externals']}==set(imports)|{'___WSAFDIsSet@8','__beginthreadex'}
for u,a in old['weak_aliases'].items():assert new['weak_aliases'][u]==a
assert all(not a for u,a in new['weak_aliases'].items() if u not in old['weak_aliases'])
assert len(new['pending'])==5
for r in new['pending']:
 p=next(p for p in old['pending'] if p['anchor']==r['anchor']);r['reference_function_code_size']=p['reference_function_code_size']
 if 'limitation' in p:r['limitation']=p['limitation']
new['status']='REVIEWED';new['sdk']=json.load(open(run/'link.json',encoding='utf-8-sig'))['sdk'];assert new['sdk']==old['sdk']
new['review']=dict(old['review']);new['review'].update(scope='TCPInterface/RakSleep4306 and font helpers331 with true BSS40012; expected4637 unique.',adaptations='Full vendor TCP class167 and real RemoteClient10 plus typed queue nodes. Whole worker/lifetimes and EH, actual Winsock/CRT boundaries. Font C++ helpers108/223 with true complete static scratch40000 and three scalars. StartupC4790 allocation remains unimplemented, no credit or runtime-ready claim.',reference_credit='Direct025 new0, cumulative21988.',verification='Only changed source units compiled, no headers. Whole functions/attached data/EH/all COFF and PE, ten controls and Actor ABI. No verifier change or redundant CRT fault tests.',excluded_candidates='Five old pending remain outside coverage. Font startup and outer hook/proxy not yet implemented. No CRT, data, allocation or duplicate code credit.')
for name,file in [('itoa_qualification','build/tcp-font-itoa-qualified.json'),('linker_alias_provider_evidence','build/cp32-tcp-font-linked1-aliases.json'),('server_bridge_final','build/agent-independent/server-bridge-target-alias-qualification-cp32-tcp-font-linked1.json'),('logger_deleting_aliases','build/agent-independent/logger-deleting-aliases-cp32-tcp-font-linked1.json'),('network_tree_deleting_aliases','build/agent-independent/replica-network-id-aliases-qualified.json'),('sha1_deleting_alias_qualification','build/tcp-font-sha1-deleting-alias-qualified.json'),('mesh_deleting_alias_qualification','build/agent-independent/mesh-deleting-aliases-cp32-tcp-font-linked1.json'),('chat_navigation_alias','build/agent-textdraw/chat-navigation-linked-alias-review.json')]:new['review'][name]=json.load(open(file))
new['review']['font_state_scope']=reviews[2]
new['review']['tcp_thread_crt_qualification']=json.load(open('build/agent-independent/tcp-thread-crt-qualified.json'))
p=Path('config/checkpoint32/tcp-font-contract.json');p.write_text(json.dumps(new,indent=2)+'\n')
g=Gate(run.name,contract_path=str(p));g.bind();new['review']['associative_comdat_bindings']={'run':run.name,'bindings':g.associative_bindings,'negative_controls':json.load(open('build/associative-binding-controls.json')),'scope':'Complete strict associative matching, no duplicate credit.'};new['review']['crt_initializer_bindings']=g.crt_initializer_bindings;p.write_text(json.dumps(new,indent=2)+'\n');print('Reviewed',len(seen),'new whole sections, all',len(prior),'old preserved')
