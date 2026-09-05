import json,sys,struct
from pathlib import Path
sys.path.insert(0,'tools')
from binary import COFF,PE
from verify_checkpoint2 import import_slots
old=json.load(open('config/checkpoint32/release-clump-contract.json'));new=json.load(open('build/actor-contract-draft.json'));prior={(r['rva'],r['size'],r['kind']):r for r in old['regions']}
pe=PE('private/samp.dll');run=Path('build/cp32-object-destructor-linked');objs={p.stem:COFF(p) for p in run.glob('*.obj')}
ids={f['symbol']:f['reference_va'] for r in old['regions'] for f in r['fixups']};ids.update({r['anchor']:pe.base+r['rva']+r['anchor_offset'] for r in old['regions']});ids.update({n:e['reference_va'] for n,e in old['externals'].items()})
# Recover member identities only from previously accepted complete source objects.
for r in old['regions']:
 o=objs.get('closure_gui_widget_lifetime' if r['unit']=='closure_gui_control_lifetime' else 'closure_dxut_state' if r['unit'] in ('closure_dxut_settings','closure_dxut_cursor','closure_dxut_multimon','closure_dxut_device') else r['unit'])
 if not o or r['kind']=='code':continue
 syms=o.names.get(r['anchor'],[]);sy=next((s for s in syms if s['section']>0),None)
 if not sy:continue
 for ss in o.names.values():
  for s in ss:
   if s['section']==sy['section'] and s['storage']==2 and not s['name'].startswith('??_C@'):
    delta=s['value']-sy['value']+r['anchor_offset']
    if 0<=delta<r['size']:ids[s['name']]=pe.base+r['rva']+delta
seeds=json.load(open('config/checkpoint32/object-destructor-seeds.json'));codes={};data_eh=set()
spec={'closure_object_destructor':[(0xa9430,300),(0xaa100,30)]}
for unit,entries in spec.items():
 for rva,size in entries:codes[rva]=(size,next(n for n,v in seeds[unit].items() if v==rva))
codes[0xe3ed0]=(18,'__ehhandler$??1R5ObjectDestructionView@@UAE@XZ')
for name,rva in seeds['closure_object_destructor'].items():ids[name]=pe.base+rva
ids['??_ER5ObjectDestructionView@@UAEPAXI@Z']=pe.base+0xaa100
reviewed_aliases={n:a['real_symbol'] for n,a in json.load(open('build/agent-textdraw/rw-parser-symbolic-aliases.json'))['aliases'].items()}
assert new['weak_aliases']['closure_rw_parser_symbolic']==reviewed_aliases
for alias,canonical in reviewed_aliases.items():ids[alias]=ids[canonical]
for unit,aliases in old['weak_aliases'].items():assert new['weak_aliases'][unit]==aliases

local={};seen=set();count=0
for r in new['regions']:
 if (r['rva'],r['size'],r['kind']) in prior:continue
 count+=1;assert r['unit'] in spec or r['unit'].startswith('closure_store_object_');o=objs[r['unit']];sec=o.sections[r['section']-1];assert sec['size']==r['size'] or (r['kind']=='script-command' and r['size']==18 and not sec['reloc_count'])
 if r['kind']=='code':
  size,prefix=codes[r['rva']];assert r['size']==size and r['anchor'].startswith(prefix);seen.add(r['rva'])
 else:
  if r['rva']==0xecd54:assert r['kind']=='data' and r['size']==18 and pe.read(r['rva'],18)==struct.pack('<H',0x108)+b'i'.ljust(16,b'\0')
  elif r['rva']==0xecd74:assert r['size']==12 and pe.read(r['rva'],12)==struct.pack('<III',0x100aa100,0x100a7db0,0x100a7e00)
  elif r['rva']==0xfbe1c:assert r['size']==36 and r['anchor_offset']==8 and pe.read(r['rva'],36)==bytes.fromhex('ffffffffd03e0e1020059319010000001cbe0f1000000000000000000000000000000000')
  else:assert r['rva'] in (0x26eb30,0x1a25f8,0x1a25fc,0x1a2600) and r['kind']=='zero' and r['size']==4
 for ss in o.names.values():
  for sy in ss:
   if sy['section']==r['section'] and r['offset']<=sy['value']<r['offset']+r['size']:
    address=pe.base+r['rva']+sy['value']-r['offset'];local[(r['unit'],sy['name'])]=address
    if sy['storage']==2:ids[sy['name']]=address
 ids[r['anchor']]=pe.base+r['rva']+r['anchor_offset']
assert seen==set(codes) and count==len(codes)+7,(count,len(codes),set(codes)-seen) # previously covered Entity provider is now linked into this closure

from verify_checkpoint2 import map_symbols
for n,e in new['externals'].items():
 if n in old['externals']:
  assert {k:v for k,v in old['externals'][n].items() if k not in ('import','library','chunks')}=={k:v for k,v in e.items() if k not in ('import','library','chunks')}
  if 'chunks' in e:assert e['chunks']==old['externals'][n]['chunks']
  for k in ('import','library','chunks'):
   if k in old['externals'][n]:e[k]=old['externals'][n][k]
 else:
  assert n=='_fread' and e==dict(kind='crt',reference_va=0x100c8482,size=76,sha256='b846334a79bf2723effce0ff2b8731139751c38c123fc7cb28d9b4450cb55d1c')
  assert any(owner.upper()=='LIBCMT:FREAD.OBJ' for _,owner in map_symbols(run/'closure.map')[n])
 ids[n]=e['reference_va']
for r in new['regions']:
 p=prior.get((r['rva'],r['size'],r['kind']))
 if p:
  assert r['sha256']==p['sha256'];assert [(f['offset'],f['kind'],f['reference_va']) for f in r['fixups']]==[(f['offset'],f['kind'],f['reference_va']) for f in p['fixups']];r['implementation']=p['implementation'];continue
 for f in r['fixups']:assert local.get((r['unit'],f['symbol']),ids.get(f['symbol']))==f['reference_va'],(r['anchor'],f)
 r['implementation']='C++' if r['kind']=='code' else 'complete packed timer/static initialization guard/input context object' if r['kind']=='zero' else 'complete source double constant'
assert len(new['pending'])==8
for r in new['pending']:
 p=next(p for p in old['pending'] if p['anchor']==r['anchor']);r['reference_function_code_size']=p['reference_function_code_size']
 if 'limitation' in p:r['limitation']=p['limitation']
for r in new['regions']:
 if (r['rva'],r['size'],r['kind']) not in prior:r['implementation']='C++' if r['kind']=='code' else 'complete source dynamic API pointer storage' if r['kind']=='zero' else 'complete original DLL/API name string'
new['status']='REVIEWED';new['sdk']=json.load(open(run/'link.json',encoding='utf-8-sig'))['sdk']
assert all(new['sdk'][n]==h for n,h in old['sdk'].items())
new['review']=dict(old['review']);new['review'].update(scope='Complete object virtual destructor and resource lifetime.',adaptations='Pointer-only derived view at CEntity base offset0. No constructor, complete object size or allocation claimed. Full actual material texture/text buffer release and shared texture ownership preserved. Four real globals in separate complete4-byte owners. Prior nonvirtual Add/Remove providers removed; new virtual providers retain exact original bodies and actual WorldAdd/WorldRemove targets.',verification='Complete300+30+18EH code, vtable12, SCRIPT_COMMAND18, FuncInfo/unwind36, four globals4. Vtable actual AA100/A7DB0/A7E00; no ambiguous deleting-destructor homologue accepted. Fullgate, ten negative controls and actor ABI required.',reference_credit='No new whole-function025 credit. Existing source skeleton only.')
new['review']['linker_alias_provider_evidence']=json.load(open('build/cp32-object-destructor-linked-aliases.json'))
assert new['weak_aliases']['closure_object_destructor']=={'??_ECEntity@@UAEPAXI@Z':'??_GCEntity@@UAEPAXI@Z','??_ER5ObjectDestructionView@@UAEPAXI@Z':'??_GR5ObjectDestructionView@@UAEPAXI@Z'}
for r in new['regions']:
 if (r['rva'],r['size'],r['kind']) not in prior or r['rva'] in (0xa7db0,0xa7e00):
  r['implementation']='C++ R5 virtual object lifetime reconstruction; compiler-generated EH' if r['kind']=='code' else 'Complete typed source vtable, descriptor, unwind metadata or4-byte owner'
Path('config/checkpoint32/object-destructor-contract.json').write_text(json.dumps(new,indent=2)+'\n')
