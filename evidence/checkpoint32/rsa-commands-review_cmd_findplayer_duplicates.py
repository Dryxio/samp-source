from pathlib import Path
import json,sys,struct,hashlib,re
sys.path.insert(0,'tools')
from binary import PE,COFF
from rank_uncovered_candidates import accepted_code
run='cp32-rsa-commands-linked1';d=Path('build')/run;p=PE(Path('private/samp.dll'));linked=PE(d/'closure.dll');base=json.loads(Path('config/checkpoint32/console-font-contract.json').read_text());draft=json.loads(Path('build/actor-contract-draft.json').read_text());assert draft['run']==run
unit='closure_cmd_save_positions';oldunit='closure_player_lifetime';rvas={0x1010,0xdfe80,0xf7610};new=[r for r in draft['regions'] if r['unit']==unit and r['rva'] in rvas];old=[r for r in base['regions'] if r['unit']==oldunit and r['rva'] in rvas];assert len(new)==len(old)==3
providers={r['anchor']:p.base+r['rva']+r.get('anchor_offset',0) for r in base['regions']+base['pending']};providers.update({n:v['reference_va'] for n,v in base['externals'].items()})
maps={}
for line in (d/'closure.map').read_text(errors='replace').splitlines():
 m=re.match(r'\s*[0-9a-fA-F]{4}:[0-9a-fA-F]{8}\s+(\S+)\s+([0-9a-fA-F]{8})\s+(?:f\s+)?(?:i\s+)?(\S+)\s*$',line)
 if m:maps.setdefault(m[1],[]).append((int(m[2],16),m[3]))
fun='?FindPlayerPed@CGame@@QAEPAVCPlayerPed@@XZ';eh='__ehhandler$'+fun
assert maps[fun]==[(maps[fun][0][0],oldunit+'.obj')] and maps[eh]==[(maps[eh][0][0],oldunit+'.obj')]
linked_bases={0x1010:maps[fun][0][0],0xdfe80:maps[eh][0][0]-11}
linked_bases[0xf7610]=struct.unpack('<I',linked.read(linked_bases[0xdfe80]-linked.base+12,4))[0]-8
rows=[];normalized={}
for owner,regions in [(oldunit,old),(unit,new)]:
 o=COFF(d/(owner+'.obj'));bysec={r['section']:r for r in regions}
 for r in regions:
  sec=o.sections[r['section']-1];assert r['offset']==0 and r['size']==sec['size'];raw=bytearray(sec['bytes']);actual=bytearray(sec['bytes']);bindings=[];lf=set();of=set()
  for i in range(sec['reloc_count']):
   off,si,k=struct.unpack_from('<IIH',o.data,sec['reloc_ptr']+10*i);s=o.symbols[si];name=s['name'];a=struct.unpack_from('<I',raw,off)[0]
   if s['section'] in bysec:
    target_region=bysec[s['section']];sv=p.base+target_region['rva']+s['value'];lv=linked_bases[target_region['rva']]+s['value']
   elif name=='__except_list':sv=lv=0
   else:
    sv=providers[name];hits=maps[name];assert len(hits)==1,(name,hits);lv=hits[0][0]
   target=(sv+a)&0xffffffff;rv=r['rva'];va=linked_bases[rv]
   struct.pack_into('<I',raw,off,(target-(p.base+rv+off+4 if k==20 else 0))&0xffffffff);struct.pack_into('<I',actual,off,(lv+a-(va+off+4 if k==20 else 0))&0xffffffff)
   if k==6 and name!='__except_list':of.add(rv+off);lf.add(va-linked.base+off)
   bindings.append(dict(symbol=name,site_rva=rv+off,symbol_va=sv,addend=a,target_va=target,kind=k,linked_target_va=lv+a))
  assert raw==p.read(r['rva'],r['size']) and hashlib.sha256(raw).hexdigest()==r['sha256'];assert actual==linked.read(linked_bases[r['rva']]-linked.base,r['size'])
  assert of=={a for a in p.relocations if r['rva']<=a<r['rva']+r['size']}
  assert lf=={a for a in linked.relocations if linked_bases[r['rva']]-linked.base<=a<linked_bases[r['rva']]-linked.base+r['size']}
  norm=[(b['site_rva']-r['rva'],b['kind'],b['target_va']) for b in bindings]
  if owner==oldunit:normalized[r['rva']]=(bytes(raw),norm)
  else:assert normalized[r['rva']]==(bytes(raw),norm)
  if owner==unit:rows.append(dict(unit=owner,anchor=r['anchor'],section=r['section'],rva=r['rva'],size=r['size'],kind=r['kind'],anchor_offset=r['anchor_offset'],sha256=r['sha256'],object_sha256=hashlib.sha256((d/(owner+'.obj')).read_bytes()).hexdigest(),bindings=bindings,selected_linked_owner=oldunit+'.obj',linked_va=linked_bases[r['rva']],new_unique_code=0))
ac=accepted_code('evidence/checkpoint32/console-font-acceptance.json');assert all(set(range(r['rva'],r['rva']+r['size']))<=ac for r in new if r['kind']=='code')
out=dict(status='THREE_DUPLICATE_WHOLE_SECTIONS_AND_ACTUAL_SELECTED_LINK_PASS',run=run,regions=rows,code_revalidated_not_new=115,data_revalidated_not_new=36,new_unique_credit=0,source='Actual inline CGame::FindPlayerPed in game/game.h, whole CPlayerPed allocation and real constructor reused; commands TU includes it and emits a COMDAT copy.',selected_owner=oldunit+'.obj',notes=['Function94, complete EH21 (handler+11), complete FuncInfo36(anchor+8) all equal seven previous original identities.','Both candidate and selected old COFF fixups independently resolve to the same whole original and actual linked bytes; PE sets equal in both directions.','No auxiliary byte or allocation receives new coverage.'])
Path('build/agent-textdraw/cmd-findplayer-duplicates-reviewed-manifest.json').write_text(json.dumps(out,indent=2)+'\n');print('PASS3 whole duplicate sections,11 fixups,115code+36data zero new credit; selected',oldunit)
