import json,sys,struct
from pathlib import Path
sys.path.insert(0,'tools')
from binary import COFF,PE,sha
from verify_actor_closure import initial_bytes,Gate
old=json.load(open('config/checkpoint32/listbox-roster-contract.json'));new=json.load(open('build/actor-contract-draft.json'));prior={(r['rva'],r['size'],r['kind']):r for r in old['regions']}
run=Path('build/cp32-database-ime-linked2');pe=PE('private/samp.dll');objs={p.stem:COFF(p) for p in run.glob('*.obj')}
reviews=[json.load(open(p)) for p in ['build/database-server-reviewed-manifest.json','build/agent-independent/database-table-final-reviewed-manifest.json','build/player-id-string-reviewed-manifest.json','build/agent-textdraw/ime-candidate-reviewed-manifest.json','build/agent-textdraw/ime-mouse-reviewed-manifest.json','build/agent-independent/player-key-codec-final-reviewed-manifest.json']]
expected={(r['unit'],r['rva'],r['size']):r for v in reviews for r in v['regions']};seen=set()
for r in new['regions']:
 p=prior.get((r['rva'],r['size'],r['kind']))
 if p:
  assert r['sha256']==p['sha256'];assert [(f['offset'],f['kind'],f['reference_va']) for f in r['fixups']]==[(f['offset'],f['kind'],f['reference_va']) for f in p['fixups']];r['implementation']=p['implementation'];continue
 key=(r['unit'],r['rva'],r['size']);assert key in expected,('Unreviewed new region',r);seen.add(key);e=expected[key]
 o=objs[r['unit']];sec=o.sections[r['section']-1];assert sec['size']==r['size'] and r['offset']==0
 assert r['sha256']==e['sha256']==sha(initial_bytes(pe,r['rva'],r['size']))
 wanted=[(f['site_rva']-r['rva'],f['kind'],f['symbol'],f['target_va']) for f in e['bindings']]
 actual=[(f['offset'],f['kind'],f['symbol'],(f['reference_va']+struct.unpack_from('<I',sec['bytes'],f['offset'])[0])&0xffffffff) for f in r['fixups']]
 assert actual==wanted,(key,actual,wanted)
 r['implementation']='Normal C++ complete vendor DatabaseServer/Table/Serializer/Common, original wide IME state/lifetimes/input and player SetKeys with true inlining context'
for key in expected:
 if key not in seen:assert any((r['rva'],r['size'])==key[1:] for r in old['regions']),('Missing reviewed section',key)
for n,e in new['externals'].items():
 if n not in old['externals']:
  assert n=='__itoa' and e['reference_va']==0x100c8b55 and e['size']==42;continue
 p=old['externals'][n];assert {k:v for k,v in p.items() if k not in ('import','library','chunks')}=={k:v for k,v in e.items() if k not in ('import','library','chunks')}
 for k in ('import','library','chunks'):
  if k in p:e[k]=p[k]
for u,a in old['weak_aliases'].items():assert new['weak_aliases'][u]==a
for u,a in new['weak_aliases'].items():
 if u not in old['weak_aliases']:assert not a
assert len(new['pending'])==5
for r in new['pending']:
 p=next(p for p in old['pending'] if p['anchor']==r['anchor']);r['reference_function_code_size']=p['reference_function_code_size']
 if 'limitation' in p:r['limitation']=p['limitation']
new['status']='REVIEWED';new['sdk']=json.load(open(run/'link.json',encoding='utf-8-sig'))['sdk'];assert new['sdk']==old['sdk']
new['review']=dict(old['review']);new['review'].update(scope='Complete DatabaseServer/Table/Common/Serializer closure plus extra table operations, PlayerID ToString, IME candidate and mouse2460, SetKeys655.',adaptations='Actual vendor sources compile exact first probes: DatabaseServer76/76, Table/Common/Serializer161/161. True Table36, Cell9, Row12, DatabaseFilter49, CellUpdate45, real BPlusTree16 pages and whole data/EH. Whole WCHAR candidate state143C and reading64, clause1024, genuine dynamic lifetime and CRT initializer. SetKeys Ob2 incorporates the actual two helpers77 moved out of old owner; GetKeys excluded and preserved separately to avoid unimplemented dependency.',reference_credit='No new direct025 credit; cumulative21947. This batch is existing vendor source plus R5 UI adaptations.',verification='Only changed owner/codec/stores recompiled; other existing objects cached with current header. Whole regions, COFF+PE targets, real pinned __itoa42/xtoa62 providers, ten controls and Actor ABI required.',excluded_candidates='GetKeys303, all old five pending and parked functions remain unaccepted. PrivateReading/IME MsgProc next probe excluded. No source bytes for CRT/native engine/virtual interface implementation.')
new['review']['crt_initializer_controls']=json.load(open('build/agent-independent/crt-initializer-binding-controls.json'))
new['review']['crt_initializer_independent_audit']=json.load(open('build/agent-independent/crt-initializer-pointer-independent-audit.json'))
new['review']['itoa_qualification']=json.load(open('build/database-ime-itoa-qualified.json'))
new['review']['linker_alias_provider_evidence']=json.load(open('build/cp32-database-ime-linked2-aliases.json'))
for name,file in [('server_bridge_final','server-bridge-target-alias-qualification-cp32-database-ime-linked2.json'),('logger_deleting_aliases','logger-deleting-aliases-cp32-database-ime-linked2.json')]:new['review'][name]=json.load(open('build/agent-independent/'+file))
new['review']['network_tree_deleting_aliases']=json.load(open('build/agent-independent/network-id-tree-deleting-aliases-cp32-database-ime-linked2.json'))
p=Path('config/checkpoint32/database-ime-contract.json');p.write_text(json.dumps(new,indent=2)+'\n')
g=Gate(run.name,contract_path=str(p));g.bind();new['review']['associative_comdat_bindings']={'run':run.name,'bindings':g.associative_bindings,'negative_controls':json.load(open('build/associative-binding-controls.json')),'scope':'Complete strict associative matching, no duplicate credit.'};new['review']['crt_initializer_bindings']=g.crt_initializer_bindings;p.write_text(json.dumps(new,indent=2)+'\n')
print('Reviewed',len(seen),'new complete regions')
