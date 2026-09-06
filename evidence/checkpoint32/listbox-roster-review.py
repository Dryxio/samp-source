import json,sys,struct
from pathlib import Path
sys.path.insert(0,'tools')
from binary import COFF,PE,sha
from verify_actor_closure import initial_bytes,Gate
old=json.load(open('config/checkpoint32/avl-string-label-contract.json'));new=json.load(open('build/actor-contract-draft.json'));prior={(r['rva'],r['size'],r['kind']):r for r in old['regions']}
run=Path('build/cp32-listbox-roster-linked2');pe=PE('private/samp.dll');objs={p.stem:COFF(p) for p in run.glob('*.obj')}
reviews=[json.load(open(p)) for p in ['build/agent-textdraw/listbox-insert287-reviewed-manifest.json','build/agent-independent/player-roster-five-final-reviewed-manifest.json']]
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
 r['implementation']='Normal C++ full ListBox item29E insertion and R5 player roster, real CNetPlayer48 and whole EH/state; true providers'
for key in expected:
 if key not in seen:assert any((r['rva'],r['size'])==key[1:] for r in old['regions']),('Missing reviewed section',key)
for n,e in new['externals'].items():
 assert n in old['externals'],('Unexpected external',n,e)
 p=old['externals'][n];assert {k:v for k,v in p.items() if k not in ('import','library','chunks')}=={k:v for k,v in e.items() if k not in ('import','library','chunks')}
 for k in ('import','library','chunks'):
  if k in p:e[k]=p[k]
for u,a in old['weak_aliases'].items():
 actual=dict(new['weak_aliases'][u])
 if u=='closure_gui_widget_lifetime':
  alias='??_ECDXUTListBox@@UAEPAXI@Z'
  assert actual.pop(alias)=='??_GCDXUTListBox@@UAEPAXI@Z'
  assert not any(f['symbol']==alias for r in new['regions'] for f in r['fixups']), 'Unreviewed ListBox deleting dependency consumed'
 assert actual==a
for u,a in new['weak_aliases'].items():
 if u not in old['weak_aliases']:assert not a
assert len(new['pending'])==5
for r in new['pending']:
 p=next(p for p in old['pending'] if p['anchor']==r['anchor']);r['reference_function_code_size']=p['reference_function_code_size']
 if 'limitation' in p:r['limitation']=p['limitation']
new['status']='REVIEWED';new['sdk']=json.load(open(run/'link.json',encoding='utf-8-sig'))['sdk'];assert new['sdk']==old['sdk']
new['review']=dict(old['review']);new['review'].update(scope='ListBox insertion287 and player roster963 including real EH63; shared header layout milestone.',adaptations='Correct ListBox actual ScrollBar5D/RECT10F/11F and InsertItem fourth color arg. Full158 owner unchanged; actual item29E allocation. Transfer RemoveAllItems73/SetSize5/SetSizeInternal142 to lifetime TU with all previous functions preserved. Roster025 source adapted to R5 CNetPlayer48, NPC and ten-byte records; true pScoreBoard4 storage moved out of main. Whole objects/functions/EH and every target.',reference_credit='025 adapted821 new principal bytes; cumulative21947. R5-only counter79 and compiler EH63 separately measured. ListBox287 no direct025 credit.',verification='Header milestone: fresh rebuild all Ob1/Ob2, old111 GUI regions preflight and final entire gate, ten controls and Actor ABI required.',excluded_candidates='ListBox ctor144/Render1014 nonexact and dependent factory/reset/VFT excluded. ServerJoin373 nonexact. Existing five pending and other parked bodies excluded; no dependent allocation credit.')
new['review']['linker_alias_provider_evidence']=json.load(open('build/cp32-listbox-roster-linked2-aliases.json'))
for name,file in [('server_bridge_final','server-bridge-target-alias-qualification-cp32-listbox-roster-linked2.json'),('logger_deleting_aliases','logger-deleting-aliases-cp32-listbox-roster-linked2.json')]:new['review'][name]=json.load(open('build/agent-independent/'+file))
new['review']['network_tree_deleting_aliases']=json.load(open('build/agent-independent/network-id-tree-deleting-aliases-cp32-listbox-roster-linked2.json'))
p=Path('config/checkpoint32/listbox-roster-contract.json');p.write_text(json.dumps(new,indent=2)+'\n')
g=Gate(run.name,contract_path=str(p));g.bind();new['review']['associative_comdat_bindings']={'run':run.name,'bindings':g.associative_bindings,'negative_controls':json.load(open('build/associative-binding-controls.json')),'scope':'Complete strict associative matching, no duplicate credit.'};p.write_text(json.dumps(new,indent=2)+'\n')
print('Reviewed',len(seen),'new complete regions')
