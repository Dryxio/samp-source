import json,sys,struct
from pathlib import Path
sys.path.insert(0,'tools')
from binary import COFF,PE,sha
from verify_checkpoint2 import import_slots
from verify_actor_closure import initial_bytes,Gate
old=json.load(open('config/checkpoint32/console-font-contract.json'));new=json.load(open('build/actor-contract-draft.json'));prior={(r['unit'],r['rva'],r['size'],r['kind']):r for r in old['regions']}
run=Path('build/cp32-rsa-commands-linked1');pe=PE('private/samp.dll');objs={p.stem:COFF(p) for p in run.glob('*.obj')}
reviews=[json.load(open(p)) for p in ['build/agent-independent/rakpeer-rsa-reviewed-manifest.json','build/agent-independent/raknet-command-parser-reviewed-manifest.json','build/agent-textdraw/cmd-save-weather-reviewed-manifest.json','build/font-ansi-consumer-reviewed.json','build/agent-textdraw/cmd-findplayer-duplicates-reviewed-manifest.json']]
reviews[2]['regions']+=reviews[2]['script_objects']
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
 else:
  allowed={'_sscanf':('crt',0xca38b,52),'_mblen':('crt',0xca3bf,137),'___mb_cur_max':('crt-data',0x117668,12)};assert n in allowed and (e['kind'],e['reference_va']-pe.base,e['size'])==allowed[n]
  if n=='___mb_cur_max':assert e['library']=='libcmt.lib' and e['member']=='nlsdata1.obj'
assert {n for n in new['externals'] if n not in old['externals']}=={'_sscanf','_mblen','___mb_cur_max'}
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
new['review']=dict(old['review']);new['review'].update(scope='RSA11013 +RakNetCommandParser5090 +commands1032 +ANSI consumer201/proxy constructor9; expected17345 unique.',adaptations='True whole RSACrypt305 instantiation including original symbolic vendor add/subtract, no allocation/VFT; PRIME_TABLE512 complete. Parser20 and VFT60 full, actual66 virtual calls distinguished with no concrete network implementation credit. Five commands C++ with actual separated script descriptor18 owners. Entire CRT locale nlsdata1.data12 bound to actual SDK block and allthree symbols; no synthetic storage. Source ANSI201 and constructor9 whose unique known VFT eliminates homologues.',reference_credit='Direct025 new280 (complete exact weather77/time162/interior41 definitions), cumulative22268. Save/Raw752 are adapted and earn zero direct025 credit. Vendor RSA/parser already existed in base.',verification='Incremental changed units only, full original/linked COFF/PE verification. New CRTdata gate: one true positive/seven in-memory negative cases, whole pinned SDK archive block12. mblen137/sscanf52 whole wrappers and true SDK targets qualified. Ten inherited controls and Actor ABI required.',excluded_candidates='Three pending source bodies still excluded, including chat descendants AddEntry/PushBack. Font startup allocation remains unimplemented. No CRT/data/duplicate source credit; no network/UI runtime execution.')
for name,file in [('itoa_qualification','build/rsa-commands-itoa-qualified.json'),('linker_alias_provider_evidence','build/cp32-rsa-commands-linked1-aliases.json'),('server_bridge_final','build/agent-independent/server-bridge-target-alias-qualification-cp32-rsa-commands-linked1.json'),('logger_deleting_aliases','build/agent-independent/logger-deleting-aliases-cp32-rsa-commands-linked1.json'),('network_tree_deleting_aliases','build/agent-independent/replica-network-id-aliases-qualified.json'),('sha1_deleting_alias_qualification','build/rsa-commands-sha1-deleting-alias-qualified.json'),('mesh_deleting_alias_qualification','build/agent-independent/mesh-deleting-aliases-cp32-rsa-commands-linked1.json'),('chat_navigation_alias','build/agent-textdraw/chat-navigation-linked-alias-review.json')]:new['review'][name]=json.load(open(file))
new['review']['rsa_scope']=json.load(open('build/agent-independent/rakpeer-rsa-scope-reviewed.json'))
new['review']['raknet_command_scope']=json.load(open('build/agent-independent/raknet-command-parser-scope-reviewed.json'))
new['review']['raknet_command_alias']=json.load(open('build/agent-independent/raknet-command-parser-alias-qualified.json'))
new['review']['console_aliases']=json.load(open('build/agent-independent/console-aliases-qualified.json'))
new['review']['new_crt_wrapper_qualification']=json.load(open('build/rsa-commands-crt-qualified.json'))
new['review']['crt_locale_data_qualification']=json.load(open('build/crt-locale-data-qualified.json'))
new['review']['crt_locale_data_controls']=json.load(open('build/crt-locale-data-controls.json'))
p=Path('config/checkpoint32/rsa-commands-contract.json');p.write_text(json.dumps(new,indent=2)+'\n')
g=Gate(run.name,contract_path=str(p));g.bind();new['review']['associative_comdat_bindings']={'run':run.name,'bindings':g.associative_bindings,'negative_controls':json.load(open('build/associative-binding-controls.json')),'scope':'Complete strict associative matching, no duplicate credit.'};new['review']['crt_initializer_bindings']=g.crt_initializer_bindings;p.write_text(json.dumps(new,indent=2)+'\n');print('Reviewed',len(seen),'new whole sections, all',len(prior),'old preserved')
