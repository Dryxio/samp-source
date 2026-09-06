import json,sys,struct
from pathlib import Path
sys.path.insert(0,'tools')
from binary import COFF,PE,sha
from verify_actor_closure import initial_bytes
old=json.load(open('config/checkpoint32/tree-insert-items-contract.json'));new=json.load(open('build/actor-contract-draft.json'));prior={(r['rva'],r['size'],r['kind']):r for r in old['regions']}
run=Path('build/cp32-logger-queries-linked3');pe=PE('private/samp.dll');objs={p.stem:COFF(p) for p in run.glob('*.obj')}
reviews=[json.load(open(p)) for p in ['build/agent-independent/packet-logger-reviewed-manifest.json','build/scoreboard-update-reviewed-normalized.json','build/agent-independent/rakserver-peer-getter-bridge-linked3-normalized-manifest.json']]
ids={f['symbol']:f['reference_va'] for r in old['regions'] for f in r['fixups']};ids.update({r['anchor']:pe.base+r['rva']+r['anchor_offset'] for r in old['regions']});ids.update({n:e['reference_va'] for n,e in old['externals'].items()})
expected={(r['unit'],r['rva'],r['size']):r for v in reviews for r in v['regions']}
for v in reviews:
 for u,names in v['seeds'].items():
  for n,rva in names.items():
   if n in ids:assert ids[n]==pe.base+rva,(n,ids[n],rva)
   ids[n]=pe.base+rva
for v in reviews:
 for n,va in v.get('external_targets',{}).items():
  if n in ids and not n.startswith('$'):assert ids[n]==va,(n,hex(ids[n]),hex(va))
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
 r['implementation']=('Explicit symbolic ABI tail bridge, no original-byte copying' if r['unit']=='closure_vendor_rakserver_internal_id_peer_bridge' else 'Complete normal C++ logger or scoreboard reconstruction with actual types and providers') if r['kind']=='code' else 'Whole original source table collection/state section, no slicing or invented contiguous placement'
for key in expected:
 if key not in seen:assert any((r['rva'],r['size'])==key[1:] for r in old['regions']),('Missing reviewed section',key)
for n,e in new['externals'].items():
 if n not in old['externals']:
  assert n=='_fputs' and e['kind']=='crt' and e['reference_va']==pe.base+0xc8c79
  from verify_actor_closure import map_symbols
  assert any('LIBCMT:fputs.obj' in owner for _,owner in map_symbols(run/'closure.map')[n])
  continue
 p=old['externals'][n];assert {k:v for k,v in p.items() if k not in ('import','library','chunks')}=={k:v for k,v in e.items() if k not in ('import','library','chunks')}
 for k in ('import','library','chunks'):
  if k in p:e[k]=p[k]
for u,a in old['weak_aliases'].items():assert new['weak_aliases'][u]==a
for v in reviews:
 for u in v['seeds']:
  if u=='closure_vendor_rakserver_internal_id_peer_bridge':continue
  expected_alias={}
  for cls in ('PacketLogger','PacketFileLogger'):
   if u==('closure_vendor_packet_logger' if cls=='PacketLogger' else 'closure_vendor_packet_file_logger'):
    expected_alias={'??_E'+cls+'@@UAEPAXI@Z':'??_G'+cls+'@@UAEPAXI@Z'}
  assert new['weak_aliases'][u]==expected_alias,(u,new['weak_aliases'][u])
assert new['weak_aliases']['closure_vendor_rakserver_internal_id_peer_bridge']=={'_r5_RakServer_InternalID_target_assembly_only':'?GetInternalID@RakServer@@UBE?AUPlayerID@@XZ'}
assert len(new['pending'])==5
for r in new['pending']:
 p=next(p for p in old['pending'] if p['anchor']==r['anchor']);r['reference_function_code_size']=p['reference_function_code_size']
 if 'limitation' in p:r['limitation']=p['limitation']
new['status']='REVIEWED';new['sdk']=json.load(open(run/'link.json',encoding='utf-8-sig'))['sdk'];assert all(new['sdk'][n]==h for n,h in old['sdk'].items())
new['review']=dict(old['review']);new['review'].update(scope='PacketLogger base/file1184 plus server ABI bridge8 and scoreboard population/playerqueries1324.',adaptations='Complete vendor logger classes10/14, Plugin4 and all selected VFTs, real server Peer+4 getter dispatch. Explicit symbolic eight-byte ABI tail bridge uses sizeof(RakServerInterface), preserves hidden PlayerID return; normal C++ method-only TU cannot emit native thunk without unavailable full server VFT. Only necessary target linker alias retained; no gate weakening. Scoreboard actual packed45-byte rows and typed queries; scratch buffer260 follows measured original stack allocation, declaration capacity257..260 not uniquely recoverable. No artificial locals or padding.',verification='Whole functions/tables/attached data and all COFF/PE targets; IDTOString whole172 compared but only118 code credited. CRT fputs is pinned library provider only. Ten negative controls and actor ABI required; unchanged compilation cache reused.',reference_credit='Direct0250, cumulative21126. Logger vendor source, scoreboard hybrid reconstruction.',initialization_limitation='No live network/game/UI or full DLL claim. Console logger and transport remain open, no empty callback assumption.',excluded_candidates='Old five pending and all parked bodies remain excluded; previous logger29 getter bytes excluded from unique gain.')
new['review']['linker_alias_provider_evidence']=json.load(open('build/cp32-logger-queries-linked3-aliases.json'))
p=Path('config/checkpoint32/logger-queries-contract.json');p.write_text(json.dumps(new,indent=2)+'\n')
from verify_actor_closure import Gate
g=Gate(run.name,contract_path=str(p));g.bind();new['review']['associative_comdat_bindings']={'run':run.name,'bindings':g.associative_bindings,'negative_controls':json.load(open('build/associative-binding-controls.json')),'scope':'Existing strict associative binding with all real bytes/targets checked and no duplicate credit.'};p.write_text(json.dumps(new,indent=2)+'\n')
print('Reviewed',len(seen),'new complete regions')

for name,file in [('server_bridge_final','server-bridge-target-alias-qualification-cp32-logger-queries-linked3.json'),('logger_deleting_aliases','logger-deleting-aliases-cp32-logger-queries-linked3.json')]:new['review'][name]=json.load(open('build/agent-independent/'+file))
p.write_text(json.dumps(new,indent=2)+'\n')
