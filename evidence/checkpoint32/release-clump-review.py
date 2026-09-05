import json,sys,struct
from pathlib import Path
sys.path.insert(0,'tools')
from binary import COFF,PE
from verify_checkpoint2 import import_slots
old=json.load(open('config/checkpoint32/object-parser-contract.json'));new=json.load(open('build/actor-contract-draft.json'));prior={(r['rva'],r['size'],r['kind']):r for r in old['regions']}
pe=PE('private/samp.dll');run=Path('build/cp32-release-clump-linked');objs={p.stem:COFF(p) for p in run.glob('*.obj')}
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
seeds=json.load(open('config/checkpoint32/release-clump-seeds.json'));codes={};data_eh=set()
spec={'closure_object_release':[(0xa09a0,141),(0xb3d30,82),(0xa8ed0,53)],'closure_empty_custom_model_release':[(0xd1d0,3)],'closure_rw_clump_slots':[(0xb97a0,27),(0xb9740,27),(0xb9760,27),(0xb9780,27),(0xb97c0,27)],'closure_rw_clump_dispatch':[(0xb97e0,310)],'closure_memory_buffer_file':[(0xba710,57),(0xba750,116),(0xba990,187)],'closure_rw_clump_file':[(0xb9920,185),(0xb99e0,116)]}
for unit,entries in spec.items():
 for rva,size in entries:codes[rva]=(size,next(n for n,v in seeds[unit].items() if v==rva))
reviewed_aliases={n:a['real_symbol'] for n,a in json.load(open('build/agent-textdraw/rw-parser-symbolic-aliases.json'))['aliases'].items()}
assert new['weak_aliases']['closure_rw_parser_symbolic']==reviewed_aliases
for alias,canonical in reviewed_aliases.items():ids[alias]=ids[canonical]
for unit,aliases in old['weak_aliases'].items():assert new['weak_aliases'][unit]==aliases

local={};seen=set();count=0
for r in new['regions']:
 if (r['rva'],r['size'],r['kind']) in prior:continue
 count+=1;assert r['unit'] in spec;o=objs[r['unit']];sec=o.sections[r['section']-1];assert sec['size']==r['size'] or (r['kind']=='script-command' and r['size']==18 and not sec['reloc_count'])
 if r['kind']=='code':
  size,prefix=codes[r['rva']];assert r['size']==size and r['anchor'].startswith(prefix);seen.add(r['rva'])
 else:
  if r['rva']==0xec6ac:assert r['kind']=='script-command' and r['size']==18 and pe.read(r['rva'],18)==struct.pack('<H',0x249)+b'i'.ljust(16,b'\0')
  else:assert r['rva']==0xe6d8c and r['size']==3 and pe.read(r['rva'],3)==b'rb\0'
 for ss in o.names.values():
  for sy in ss:
   if sy['section']==r['section'] and r['offset']<=sy['value']<r['offset']+r['size']:
    address=pe.base+r['rva']+sy['value']-r['offset'];local[(r['unit'],sy['name'])]=address
    if sy['storage']==2:ids[sy['name']]=address
 ids[r['anchor']]=pe.base+r['rva']+r['anchor_offset']
assert seen==set(codes) and count==len(codes)+2,(count,len(codes),set(codes)-seen) # previously covered Entity provider is now linked into this closure

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
new['review']=dict(old['review']);new['review'].update(scope='Complete object model/material release and RenderWare clump file dependencies.',adaptations='Model release empty provider is original whole D1D0 ret4, kept in separate source unit to preserve true call. Five slot setters and clump185 use original positive-case branch ordering. Complete 310-byte dispatcher retains 50 attached table bytes, with only260 code bytes credited. Buffer23/chunk35 storage preserved. SDK texture IUnknown Release identity verified through real SetTexture consumer. No native execution.',verification='All whole COFF sections, typed command18, rb3 string and all provider targets checked. New fread76 pinned original entry and actual LIBCMT:fread.obj owner, zero CRT code credit. Existing aliases reverified in current linked image.',reference_credit='No new whole-function025 credit; existing native call fragment aids RemoveModel only.')
new['review']['linker_alias_provider_evidence']=json.load(open('build/cp32-release-clump-linked-aliases.json'))
for r in new['regions']:
 if (r['rva'],r['size'],r['kind']) not in prior:
  r['implementation']=('C++ with labeled existing symbolic native/pool access fragment' if r['rva'] in (0xa09a0,0xb3d30) else 'C++ reconstructed R5 whole function') if r['kind']=='code' else 'Complete source SCRIPT_COMMAND18 or rb string3'
assert next(r for r in new['regions'] if r['rva']==0xb97e0)['code_ranges']==[dict(offset=0,size=260)]
Path('config/checkpoint32/release-clump-contract.json').write_text(json.dumps(new,indent=2)+'\n')
