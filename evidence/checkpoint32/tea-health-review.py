import json,sys,struct
from pathlib import Path
sys.path.insert(0,'tools')
from binary import COFF,PE,sha
from verify_actor_closure import initial_bytes
old=json.load(open('config/checkpoint32/reliability-preview-contract.json'));new=json.load(open('build/actor-contract-draft.json'));prior={(r['rva'],r['size'],r['kind']):r for r in old['regions']}
run=Path('build/cp32-tea-health-linked1');pe=PE('private/samp.dll');objs={p.stem:COFF(p) for p in run.glob('*.obj')}
reviews=[json.load(open('build/agent-independent/tea-reviewed-manifest.json')),json.load(open('build/agent-textdraw/player-tag-health-reviewed-manifest.json'))]
ids={f['symbol']:f['reference_va'] for r in old['regions'] for f in r['fixups']};ids.update({r['anchor']:pe.base+r['rva']+r['anchor_offset'] for r in old['regions']});ids.update({n:e['reference_va'] for n,e in old['externals'].items()})
expected={(r['unit'],r['rva'],r['size']):r for v in reviews for r in v['regions']}
for v in reviews:
 for u,names in v['seeds'].items():
  for n,rva in names.items():
   if n in ids:assert ids[n]==pe.base+rva,(n,ids[n],rva)
   ids[n]=pe.base+rva
for v in reviews:
 for n,va in v.get('external_targets',{}).items():
  if n in ids:assert ids[n]==va,(n,hex(ids[n]),hex(va))
seen=set()
for r in new['regions']:
 p=prior.get((r['rva'],r['size'],r['kind']))
 if p:
  assert r['sha256']==p['sha256'];assert [(f['offset'],f['kind'],f['reference_va']) for f in r['fixups']]==[(f['offset'],f['kind'],f['reference_va']) for f in p['fixups']];r['implementation']=p['implementation'];continue
 key=(r['unit'],r['rva'],r['size']);assert key in expected,('Unreviewed new region',r);seen.add(key);e=expected[key]
 o=objs[r['unit']];sec=o.sections[r['section']-1];assert sec['size']==r['size'] and r['offset']==0
 assert r['sha256']==e['sha256'];assert sha(initial_bytes(pe,r['rva'],r['size']))==e['sha256']
 wanted=[(f['site_rva']-r['rva'],f['kind'],f['symbol'],f['target_va']) for f in e['bindings']]
 actual=[(f['offset'],f['kind'],f['symbol'],(f['reference_va']+struct.unpack_from('<I',sec['bytes'],f['offset'])[0])&0xffffffff) for f in r['fixups']]
 assert actual==wanted,(key,actual,wanted)
 r['implementation']='Complete normal C++ with real TEA/DataBlock or player-tag class, actual COM interface/native import contracts' if r['kind']=='code' else 'Whole original source table collection/state section, no slicing or invented contiguous placement'
for key in expected:
 if key not in seen:assert any((r['rva'],r['size'])==key[1:] for r in old['regions']),('Missing reviewed section',key)
for n,e in new['externals'].items():
 if n not in old['externals']:
  assert n=='_D3DXVec3Project@24' and e['kind']=='import-thunk' and e['reference_va']==pe.base+0xc5c80 and e['import_slot_va']==pe.base+0xe5514 and e['size']==6 and e['import_identity']==['d3dx9_25.dll','D3DXVec3Project']
  assert e['sha256']==sha(pe.read(0xc5c80,6))
  assert struct.unpack_from('<I',pe.read(0xc5c80,6),2)[0]==pe.base+0xe5514
  from verify_checkpoint2 import import_slots
  assert import_slots(pe)[pe.base+0xe5514]==('d3dx9_25.dll','D3DXVec3Project')
  lines=(run/'closure.map').read_text().splitlines();assert any('_D3DXVec3Project@24' in line and 'd3dx9:d3dx9_25.dll' in line for line in lines)
  e['library']='d3dx9.lib'
  continue
 p=old['externals'][n];assert {k:v for k,v in p.items() if k not in ('import','library','chunks')}=={k:v for k,v in e.items() if k not in ('import','library','chunks')}
 for k in ('import','library','chunks'):
  if k in p:e[k]=p[k]
for u,a in old['weak_aliases'].items():assert new['weak_aliases'][u]==a
assert all(not new['weak_aliases'][u] for v in reviews for u in v['seeds'])
assert len(new['pending'])==5
for r in new['pending']:
 p=next(p for p in old['pending'] if p['anchor']==r['anchor']);r['reference_function_code_size']=p['reference_function_code_size']
 if 'limitation' in p:r['limitation']=p['limitation']
new['status']='REVIEWED';new['sdk']=json.load(open(run/'link.json',encoding='utf-8-sig'))['sdk'];assert all(new['sdk'][n]==h for n,h in old['sdk'].items())
new['review']=dict(old['review']);new['review'].update(scope='Complete TEABlockEncryptor1358 plus player health/armor rendering and matrix hook1972.',adaptations='TEA normal vendor C++ exact first probe, true610-byte derived class and DataBlock586 base. Preserve source no-op SetKey and byte-index key behavior. Health normal base C++ six methods exact Ob1, Begin217 differed because Restore was not inlined; genuine whole TU Ob2 matches all seven bodies including Begin246 and Draw1456. Other units stay Ob1. Actual class12, source COM lifecycle and actual hook method, no fake vtable allocation.',verification='Whole functions and complete source data, all COFF/PE targets. TEA initObsDelta4, three vertex arrays80, three matrix64 and float4 constants. Real d3dx9_25.dll import thunk/slot linked from pinned import library, no thunk code credit. Previously accepted DataBlock ctor10/dtor1 providers reused with zero duplicate credit. Ten controls and actor ABI required. No new headers; unchanged objects reused.',reference_credit='Existing baseline/vendor source, direct0250 for all this batch; older025 tags renderer differs and gives no incremental source advantage here.',initialization_limitation='Real TEA/DataBlock class layouts and full CNewPlayerTags lifecycle; only actual hook method covered, full device-hook vtable is not claimed. No native rendering, packet/network or game execution; whole DLL incomplete.',excluded_candidates='No unused CheckSum/D3DX helper homologues seeded. TEA base empty destructor follows actual original TEA caller, not arbitrary same-byte inference. Dynamic D3DX thunk excluded from source coverage.')
new['review']['linker_alias_provider_evidence']=json.load(open('build/cp32-tea-health-linked1-aliases.json'))
p=Path('config/checkpoint32/tea-health-contract.json');p.write_text(json.dumps(new,indent=2)+'\n')
from verify_actor_closure import Gate
g=Gate(run.name,contract_path=str(p));g.bind();new['review']['associative_comdat_bindings']={'run':run.name,'bindings':g.associative_bindings,'negative_controls':json.load(open('build/associative-binding-controls.json')),'scope':'Existing strict associative binding with all real bytes/targets checked and no duplicate credit.'};p.write_text(json.dumps(new,indent=2)+'\n')
print('Reviewed',len(seen),'new complete regions')
