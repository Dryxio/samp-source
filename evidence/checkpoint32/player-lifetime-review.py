import json,sys,struct
from pathlib import Path
sys.path.insert(0,'tools')
from binary import COFF,PE
from verify_checkpoint2 import import_slots
old=json.load(open('config/checkpoint32/object-destructor-contract.json'));new=json.load(open('build/actor-contract-draft.json'));prior={(r['rva'],r['size'],r['kind']):r for r in old['regions']}
pe=PE('private/samp.dll');run=Path('build/cp32-player-lifetime-linked');objs={p.stem:COFF(p) for p in run.glob('*.obj')}
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
seeds=json.load(open('config/checkpoint32/player-lifetime-seeds.json'));codes={};data_eh=set()
spec={'closure_player_lifetime':[(0xaba70,308),(0xb12a0,70),(0xb13d0,30),(0x1010,94),(0xb0fa0,312),(0xadc00,141),(0xaee30,28)],'closure_player_attachments_symbolic':[(0xb0ab0,86)]}
for unit,entries in spec.items():
 for rva,size in entries:codes[rva]=(size,next(n for n,v in seeds[unit].items() if v==rva))
for rva,size,parent in [(0xdfe80,21,0x1010),(0xe3ef0,18,0xaba70),(0xe3f70,18,0xb12a0)]:codes[rva]=(size,'__ehhandler$'+codes[parent][1])
for unit in spec:
 for name,rva in seeds[unit].items():ids[name]=pe.base+rva
ids['??_ECPlayerPed@@UAEPAXI@Z']=pe.base+0xb13d0
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
  commands={0xecfc4:(0x87e,b'ii'),0xece84:(0x446,b'ii'),0xed050:(0x682,b'ifffi'),0xed03c:(0x9a2,b'i'),0xed12c:(0x70a,b'iifffiissi')}
  if r['rva'] in commands:
   op,args=commands[r['rva']];assert r['size']==18 and pe.read(r['rva'],18)==struct.pack('<H',op)+args.ljust(16,b'\0')
  elif r['rva']==0xed140:assert r['size']==12 and pe.read(r['rva'],12)==struct.pack('<III',0x100b13d0,0x1009f300,0x1009f400)
  elif r['rva']==0xed210:assert r['size']==5 and pe.read(r['rva'],5)==b'NULL\0'
  else:
   handlers={0xf7610:0xdfe80,0xfbe40:0xe3ef0,0xfbed0:0xe3f70};assert r['size']==36 and r['anchor_offset']==8 and r['rva'] in handlers
   assert pe.read(r['rva'],36)==struct.pack('<IIIIIIIII',0xffffffff,pe.base+handlers[r['rva']],0x19930520,1,pe.base+r['rva'],0,0,0,0)
 for ss in o.names.values():
  for sy in ss:
   if sy['section']==r['section'] and r['offset']<=sy['value']<r['offset']+r['size']:
    address=pe.base+r['rva']+sy['value']-r['offset'];local[(r['unit'],sy['name'])]=address
    if sy['storage']==2:ids[sy['name']]=address
 ids[r['anchor']]=pe.base+r['rva']+r['anchor_offset']
assert seen==set(codes) and count==len(codes)+10,(count,len(codes),set(codes)-seen) # previously covered Entity provider is now linked into this closure

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
new['review']=dict(old['review']);new['review'].update(scope='Player wrapper construction and complete destruction/attached object lifetime.',adaptations='Seven principal functions exact first C++ trial. RemoveAll86 is labeled symbolic exception after three normal C++ variants selected the wrong induction base. Full exact 32D object size/base48/field offsets compile assertions; header only adds methods and override, no fields. FindPlayerPed uses genuine original32D allocation and ctorABA70. No artificial factory credit. Native GTA deletion uses existing025 symbolic source protocol with actual native pointer.',verification='All whole bodies, EH21+18+18, full36-byte maps, actual vtableED140 to B13D0/9F300/9F400, five complete18-byte SCRIPT_COMMAND objects and NULL5 string. Static decapitated descriptorECE84 remains distinct from accepted ActorPedEC280. Virtual attached-object deletion uses accepted ECD74 object chain, not a fake CObject size. Fullgate, ten negative controls and actorABI required.',reference_credit='312 adapted025 Destroy source bytes only; ctor and FindPlayerPed already in base. Other R5 helpers and compilerEH do not receive025 credit.')
new['review']['linker_alias_provider_evidence']=json.load(open('build/cp32-player-lifetime-linked-aliases.json'))
assert new['weak_aliases']['closure_player_lifetime']=={'??_ECEntity@@UAEPAXI@Z':'??_GCEntity@@UAEPAXI@Z','??_ECPlayerPed@@UAEPAXI@Z':'??_GCPlayerPed@@UAEPAXI@Z'}
for r in new['regions']:
 if (r['rva'],r['size'],r['kind']) not in prior:
  r['implementation']=('Explicit R5 symbolic exception after three failed normal C++ induction layouts' if r['rva']==0xb0ab0 else 'C++ adapted025 Destroy with existing symbolic native destruction protocol' if r['rva']==0xb0fa0 else 'C++ whole R5/base player lifetime, complete compiler EH') if r['kind']=='code' else 'Complete typed vtable, source descriptor/string or unwind metadata'
Path('config/checkpoint32/player-lifetime-contract.json').write_text(json.dumps(new,indent=2)+'\n')
