import sys,json,struct,hashlib,argparse
from pathlib import Path
sys.path.insert(0,'tools')
from binary import PE,COFF
from rank_uncovered_candidates import accepted_code
ap=argparse.ArgumentParser();ap.add_argument('--run',required=True);args=ap.parse_args()
p=PE(Path('private/samp.dll'));run=args.run;d=Path('build')/run
contract=json.load(open('config/checkpoint32/nat-chat-mesh-contract.json'))
providers={r['anchor']:p.base+r['rva']-r.get('anchor_offset',0) for r in contract['regions']};providers.update({n:v['reference_va'] for n,v in contract['externals'].items()})
callback=COFF(Path('build/cp32-listbox-roster-ob1/closure_ime_callbacks.obj'))
cr=next(r for r in contract['regions'] if r['unit']=='closure_ime_callbacks')
cs=next(s for s in callback.names[cr['anchor']] if s['section']>0)
for sym in callback.symbols.values():
 if sym['section']==cs['section'] and sym['storage']==2:
  providers[sym['name']]=p.base+cr['rva']+sym['value']
providers['?s_adwCompStringClause@CDXUTIMEEditBox@@1PAKA']=p.base+0x141928
new={'?r5ImeCandidateState@@3UR5ImeCandidateListState@@A':0x1427b0,'?r5ImeReadingString@@3PAGA':0x142530,'??0R5ImeCandidateListState@@QAE@XZ':0x85980,'??1R5ImeCandidateListState@@QAE@XZ':0x859a0,'_$E5':0xe42f0,'_$E6':0xe4380};providers.update({n:p.base+r for n,r in new.items()})
interaction='closure_chat_ime_interaction';nav='closure_chat_page_navigation'
unitdirs={interaction:Path('build/cp32-replica-navigation-trial1'),nav:d}
plan=[(interaction,'?IsActive@R5ImeInteractionView@@SA_NXZ',0x863e0,42,'code'),(interaction,'?IsImeInteractionActive@R5ChatInputQueryView@@QAE_NXZ',0x69810,15,'code'),(nav,'?PageUp@R5ChatPageNavigationView@@QAEXXZ',0x672d0,82,'code'),(nav,'?PageDown@R5ChatPageNavigationView@@QAEXXZ',0x67330,91,'code'),(nav,'?R5SetTrackPos@R5ChatScrollBarNavigationView@@QAEXH@Z',0x670e0,29,'code')]
providers.update({n:p.base+r for _,n,r,_,_ in plan})
alias='_r5ImeShowReadingWindowImport';target='?s_bShowReadingWindow@CDXUTIMEEditBox@@1_NA'
providers[alias]=providers[target]
assert providers[alias]==p.base+0x141806
io=COFF(unitdirs[interaction]/(interaction+'.obj')); directives=b''.join(s['bytes'] for s in io.sections if s['name']=='.drectve').decode('ascii')
assert '/alternatename:'+alias+'='+target in directives
assert all(sym['section']<=0 for sym in io.names[alias]),'alias must never define storage'
# New imports are resolved from the original import directory, never discovery guesses.
def readstr(rva):
 out=bytearray()
 while p.read(rva+len(out),1)!=b'\0':out.extend(p.read(rva+len(out),1))
 return out.decode('ascii')
opt=struct.unpack_from('<I',p.data,0x3c)[0]+24
irva=struct.unpack_from('<I',p.data,opt+104)[0];imports={}
while True:
 ilt,_,_,dll,iat=struct.unpack('<IIIII',p.read(irva,20))
 if not any((ilt,dll,iat)):break
 index=0
 while True:
  entry=struct.unpack('<I',p.read((ilt or iat)+4*index,4))[0]
  if not entry:break
  if not entry&0x80000000:imports[readstr(entry+2)]={'dll':readstr(dll),'reference_va':p.base+iat+4*index}
  index+=1
 irva+=20
new_imports={}
for objunit in {t[0] for t in plan}:
 obj=COFF(unitdirs[objunit]/(objunit+'.obj'))
 for sym in obj.symbols.values():
  name=sym['name']
  if name.startswith('__imp__') and name not in providers:
   api=name[7:].split('@')[0]
   info=imports[api];providers[name]=info['reference_va'];new_imports[name]=dict(info,library=info['dll'].replace('.dll','.lib'))
rows=[];union=set()
for u,n,r,z,kind in plan:
 o=COFF(unitdirs[u]/(u+'.obj'));s=next(s for s in o.names[n] if s['section']>0);sec=o.sections[s['section']-1];assert s['value']==0 and sec['size']==z,(n,s,sec['size']);raw=bytearray(bytes(z) if sec['uninitialized'] else sec['bytes']);bs=[]
 for i in range(sec['reloc_count']):
  off,si,k=struct.unpack_from('<IIH',o.data,sec['reloc_ptr']+i*10);ref=o.symbols[si];name=ref['name'];a=struct.unpack_from('<I',raw,off)[0];sv=(p.base+r+ref['value'] if ref['section']==s['section'] else providers[name]);target=(sv+a)&0xffffffff;site=r+off;orig=struct.unpack('<I',p.read(site,4))[0]
  assert target==(orig if k==6 else (orig+p.base+site+4)&0xffffffff),(n,name,hex(target),hex(orig))
  struct.pack_into('<I',raw,off,(target-(p.base+site+4 if k==20 else 0))&0xffffffff)
  bs.append(dict(symbol=name,site_rva=site,symbol_va=sv,addend=a,target_va=target,kind=k))
 if sec['uninitialized']:
  ps=next(q for q in p.sections if q['rva']<=r<q['rva']+q['virtual_size']);assert r>=ps['rva']+ps['size'] and r+z<=ps['rva']+ps['virtual_size'];original=bytes(z)
 else:original=p.read(r,z)
 assert raw==original,(n,'bytes');assert {b['site_rva'] for b in bs if b['kind']==6 and b['symbol']!='__except_list'}=={x for x in p.relocations if r<=x<r+z},(n,'PE relocations')
 if kind in ('code','code_and_tables'):union.update(range(r,r+(2458 if kind=='code_and_tables' else z)))
 rows.append(dict(unit=u,anchor=n,rva=r,size=z,section=s['section'],kind=kind,object_sha256=hashlib.sha256((unitdirs[u]/(u+'.obj')).read_bytes()).hexdigest(),sha256=hashlib.sha256(raw).hexdigest(),whole_bytes_equal=True,bindings=bs))
a=accepted_code('evidence/checkpoint32/connection-graph-acceptance.json')
assert len(a)==334455 and len(union)==259 and len(union-a)==259
result=dict(status='FULL_OBJECT_PREFLIGHT_ACTUAL_LINK_ALIAS_PENDING',run=run,baseline=len(a),code_bytes=259,new_unique_code=259,EH=0,data_bytes=0,direct025_code_credit=0,regions=rows,new_imports=new_imports,import_alias={alias:'?s_bShowReadingWindow@CDXUTIMEEditBox@@1_NA'},alias_original_va=providers[alias],excluded=['GetTrackPos7 is inline in navigation; no separate credit.', 'SDK SetTrackPos COMDAT23 remains unrelated; navigation owns distinct R5SetTrackPos29.'])
Path('build/agent-textdraw/chat-navigation-reviewed-manifest.json').write_text(json.dumps(result,indent=2)+'\n')
Path('build/agent-textdraw/chat-navigation-seeds.json').write_text(json.dumps([dict(unit=u,symbol=n,rva=r,size=z) for u,n,r,z,_ in plan],indent=2)+'\n')
print('PASS',len(rows),'whole sections',sum(len(r['bindings']) for r in rows),'bindings;259code;0data/EH;true data alias link pending')
