import json,sys,struct
from pathlib import Path
sys.path.insert(0,'tools')
from binary import COFF,PE,sha
from verify_checkpoint2 import import_slots
from verify_actor_closure import initial_bytes,Gate
old=json.load(open('config/checkpoint32/tcp-font-contract.json'));new=json.load(open('build/actor-contract-draft.json'));prior={(r['unit'],r['rva'],r['size'],r['kind']):r for r in old['regions']}
run=Path('build/cp32-console-font-linked1');pe=PE('private/samp.dll');objs={p.stem:COFF(p) for p in run.glob('*.obj')}
reviews=[json.load(open(p)) for p in ['build/agent-independent/console-reviewed-manifest.json','build/agent-textdraw/font-hook-complete-reviewed-manifest.json','build/agent-textdraw/font-ansi-final-reviewed-manifest.json']]
reviews[2]['regions']=[reviews[2]['parser']]
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
for n,e in new['externals'].items():
 if n in old['externals']:
  p=old['externals'][n];assert {k:v for k,v in p.items() if k not in ('import','library','chunks')}=={k:v for k,v in e.items() if k not in ('import','library','chunks')}
  for k in ('import','library','chunks'):
   if k in p:e[k]=p[k]
 else:assert n=='_strtoul' and e['kind']=='crt' and e['reference_va']==pe.base+0xc7df5 and e['size']==23
assert {n for n in new['externals'] if n not in old['externals']}=={'_strtoul'}
for u,a in old['weak_aliases'].items():assert new['weak_aliases'][u]==a
for u,a in new['weak_aliases'].items():
 if u not in old['weak_aliases']:assert a==reviews[0]['actual_weak_aliases'].get(u,{})
assert len(new['pending'])==3
for r in new['pending']:
 p=next(p for p in old['pending'] if p['anchor']==r['anchor']);r['reference_function_code_size']=p['reference_function_code_size']
 if 'limitation' in p:r['limitation']=p['limitation']
new['status']='REVIEWED';new['sdk']=json.load(open(run/'link.json',encoding='utf-8-sig'))['sdk'];assert new['sdk']==old['sdk']
new['review']=dict(old['review']);new['review'].update(scope='Console/parser/Telnet/logger8195 plus Font439 and ANSI271, expected8905 unique.',adaptations='Complete vendor console family and true TCP providers, real Console1040/Parser16/Log160/Telnet20. Entire static proxy4 and actual COM VFT56, all14 implementations. Sole FontHook owner replaces pending16 with complete229, all17 siblings preserved. ANSI parser271 renders first hex predicate inline as original, otherfive calls retained, all53 old util sections preserved. True pointer stores8, no source data credit.',reference_credit='Direct025 new0, cumulative21988.',verification='Changed source only, no header/verifier change. Whole source/data/EH and all targets/PE, real aliases, ten controls and Actor ABI required. New strtoul23 fully qualified to actual strtoxl SDK target.',excluded_candidates='Three old pending remain outside coverage. Font startupC4790 heap allocations not implemented or counted. Unused proxy constructor9 excluded. No CRT/data/duplicate credit.')
for name,file in [('itoa_qualification','build/console-font-itoa-qualified.json'),('linker_alias_provider_evidence','build/cp32-console-font-linked1-aliases.json'),('server_bridge_final','build/agent-independent/server-bridge-target-alias-qualification-cp32-console-font-linked1.json'),('logger_deleting_aliases','build/agent-independent/logger-deleting-aliases-cp32-console-font-linked1.json'),('network_tree_deleting_aliases','build/agent-independent/replica-network-id-aliases-qualified.json'),('sha1_deleting_alias_qualification','build/console-font-sha1-deleting-alias-qualified.json'),('mesh_deleting_alias_qualification','build/agent-independent/mesh-deleting-aliases-cp32-console-font-linked1.json'),('chat_navigation_alias','build/agent-textdraw/chat-navigation-linked-alias-review.json')]:new['review'][name]=json.load(open(file))
new['review']['font_state_scope']=dict(scope='Static object4/VFT56 and true scalar stores8; startup allocator remains outside coverage.',font_manifest=reviews[1]['status'],ansi_manifest=reviews[2]['status'])
new['review']['console_aliases']=json.load(open('build/agent-independent/console-aliases-qualified.json'))
new['review']['strtoul_qualification']=json.load(open('build/console-font-strtoul-qualified.json'))
new['review']['tcp_thread_crt_qualification']=json.load(open('build/agent-independent/tcp-thread-crt-qualified.json'))
p=Path('config/checkpoint32/console-font-contract.json');p.write_text(json.dumps(new,indent=2)+'\n')
g=Gate(run.name,contract_path=str(p));g.bind();new['review']['associative_comdat_bindings']={'run':run.name,'bindings':g.associative_bindings,'negative_controls':json.load(open('build/associative-binding-controls.json')),'scope':'Complete strict associative matching, no duplicate credit.'};new['review']['crt_initializer_bindings']=g.crt_initializer_bindings;p.write_text(json.dumps(new,indent=2)+'\n');print('Reviewed',len(seen),'new whole sections, all',len(prior),'old preserved')
