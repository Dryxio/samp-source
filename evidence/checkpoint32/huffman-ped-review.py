import json,sys,struct
from pathlib import Path
sys.path.insert(0,'tools')
from binary import COFF,PE,sha
old=json.load(open('config/checkpoint32/death-vehicle-contract.json'));new=json.load(open('build/actor-contract-draft.json'));prior={(r['rva'],r['size'],r['kind']):r for r in old['regions']}
run=Path('build/cp32-huffman-ped-linked1');pe=PE('private/samp.dll');objs={p.stem:COFF(p) for p in run.glob('*.obj')}
vendor=json.load(open('build/agent-independent/vendor-compression-reviewed-manifest.json'));ped=json.load(open('build/agent-textdraw/ped-pair-full-preflight.json'))
ids={f['symbol']:f['reference_va'] for r in old['regions'] for f in r['fixups']};ids.update({r['anchor']:pe.base+r['rva']+r['anchor_offset'] for r in old['regions']});ids.update({n:e['reference_va'] for n,e in old['externals'].items()})
expected={}
for r in vendor['regions']:
 expected[(r['unit'],r['rva'],r['size'])]=r
for r in ped['full_sections']:
 expected[(r['unit'],int(r['rva'],16),r['size'])]=r
for u,names in vendor['seeds'].items():
 for n,rva in names.items():
  if n in ids:assert ids[n]==pe.base+rva,(n,ids[n],rva)
  ids[n]=pe.base+rva
for n,va in vendor['external_targets'].items():
 if n in ids:assert ids[n]==va,(n,hex(ids[n]),hex(va))
for b in ped['bindings']:
 if b['symbol'] in ids:assert ids[b['symbol']]==int(b['actual_target'],16),(b,ids[b['symbol']])
payloads={(r['unit'],int(r['rva'],16),r['size']):r for r in ped['payloads']}
seen=set()
for r in new['regions']:
 p=prior.get((r['rva'],r['size'],r['kind']))
 if p:
  assert r['sha256']==p['sha256'];assert [(f['offset'],f['kind'],f['reference_va']) for f in r['fixups']]==[(f['offset'],f['kind'],f['reference_va']) for f in p['fixups']];r['implementation']=p['implementation'];continue
 o=objs[r['unit']];sec=o.sections[r['section']-1];assert sec['size']==r['size'] or (r['kind']=='script-command' and r['size']==18 and not sec['reloc_count'])
 key=(r['unit'],r['rva'],r['size']);e=expected.get(key)
 if e:
  seen.add(key)
  if 'sha256' in e:assert r['sha256']==e['sha256']
  if 'bindings' in e:
   wanted=[(f['site_rva']-r['rva'],f['kind'],f['symbol'],f['target_va']) for f in e['bindings']]
   actual=[(f['offset'],f['kind'],f['symbol'],(f['reference_va']+struct.unpack_from('<I',sec['bytes'],r['offset']+f['offset'])[0])&0xffffffff) for f in r['fixups']]
  else:
   wanted=[(f['offset'],f['kind'],f['symbol'],int(f['provider_va'],16)) for f in e['fixups']]
   actual=[(f['offset'],f['kind'],f['symbol'],f['reference_va']) for f in r['fixups']]
  assert actual==wanted,(key,actual,wanted)
 elif key in payloads:
  assert pe.read(r['rva'],r['size'])==bytes.fromhex(payloads[key]['bytes']);assert not r['fixups']
 elif key==('closure_store_english_frequencies',0x101de8,1024):
  assert r['anchor']=='?englishCharacterFrequencies@@3PAIA' and r['anchor_offset']==0 and not r['fixups'];assert r['sha256']==vendor['whole_data_objects'][0]['sha256']
 elif key==('closure_vendor_string_compressor',0x121a3c,8):
  assert r['kind']=='zero' and r['anchor_offset'] in (0,4) and not r['fixups'];assert r['sha256']==sha(bytes(8))
 else:raise AssertionError(('Unreviewed region',r))
 r['implementation']='Complete normal C++ source and compiler-generated whole EH; vendor source uses true Queue/LinkedList/Map lifetimes, ped uses full actual CPlayerPed layout' if r['kind']=='code' else 'Complete actual source storage, typed SCRIPT_COMMAND or compiler unwind metadata'
# Previously accepted regions can also coincide with reviewed vendor helper code; no duplicate credit.
for key in expected:
 if key not in seen:assert any((r['rva'],r['size'])==key[1:] for r in old['regions']),('Missing reviewed section',key)
for n,e in new['externals'].items():
 assert n in old['externals'],('Unexpected external',n,e)
 p=old['externals'][n];assert {k:v for k,v in p.items() if k not in ('import','library','chunks')}=={k:v for k,v in e.items() if k not in ('import','library','chunks')}
 for k in ('import','library','chunks'):
  if k in p:e[k]=p[k]
for u,aliases in old['weak_aliases'].items():assert new['weak_aliases'][u]==aliases
assert len(new['pending'])==5
for r in new['pending']:
 p=next(p for p in old['pending'] if p['anchor']==r['anchor']);r['reference_function_code_size']=p['reference_function_code_size']
 if 'limitation' in p:r['limitation']=p['limitation']
new['status']='REVIEWED';new['sdk']=json.load(open(run/'link.json',encoding='utf-8-sig'))['sdk'];assert all(new['sdk'][n]==h for n,h in old['sdk'].items())
new['review']=dict(old['review']);new['review'].update(scope='Full Huffman/StringCompressor closure and actual remote ped constructor694 + ped preview634, whole compiler EH and data.',adaptations='Normal C++ exact first probe. English frequency array isolated into minimal real store TU preserving external unsigned int[256]. Four whole internal SCRIPT_COMMAND records explicitly approved. True CPlayerPed813 ctor and existing virtual lifetime; no view allocation.',verification='Every complete region and relocation compared; vendor template identities follow named original callers, no arbitrary homologue. Whole English table1024, zero globals8, two script stores18, all EH and literals verified. Prior coverage remains preserved; ten controls and actor ABI required.',reference_credit='No direct025 whole-body incremental credit. Vendor implementations already existed in base.025 supplied useful missing ped lifecycle structure and script descriptors; constructor is substantially R5-adapted.',initialization_limitation='Real ped constructor/lifetime accepted, renderer calls native game code but is never executed in game. Vehicle and object preview branches remain incomplete.Whole DLL false.',huffman_ctor_identity=vendor['special_case'])
new['review']['linker_alias_provider_evidence']=json.load(open('build/cp32-huffman-ped-linked1-aliases.json'))
p=Path('config/checkpoint32/huffman-ped-contract.json');p.write_text(json.dumps(new,indent=2)+'\n')
from verify_actor_closure import Gate
g=Gate(run.name,contract_path=str(p));g.bind();new['review']['associative_comdat_bindings']={'run':run.name,'bindings':g.associative_bindings,'negative_controls':json.load(open('build/associative-binding-controls.json')),'scope':'Existing strict associative ownership binding; all actual bytes and targets verified without duplicate credit.'};p.write_text(json.dumps(new,indent=2)+'\n')
print('Reviewed',len(new['regions']),'regions')
