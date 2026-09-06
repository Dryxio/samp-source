import sys,json,struct,hashlib
from pathlib import Path
sys.path.insert(0,'tools')
from binary import PE,COFF
from rank_uncovered_candidates import accepted_code
p=PE(Path('private/samp.dll'));run='cp32-menu-process-trial2';d=Path('build')/run
contract=json.load(open('config/checkpoint32/database-ime-contract.json'))
providers={r['anchor']:p.base+r['rva']-r.get('anchor_offset',0) for r in contract['regions']};providers.update({n:v['reference_va'] for n,v in contract['externals'].items()})
callback=COFF(Path('build/cp32-listbox-roster-ob1/closure_ime_callbacks.obj'))
cr=next(r for r in contract['regions'] if r['unit']=='closure_ime_callbacks')
cs=next(s for s in callback.names[cr['anchor']] if s['section']>0)
for sym in callback.symbols.values():
 if sym['section']==cs['section'] and sym['storage']==2:
  providers[sym['name']]=p.base+cr['rva']+sym['value']
providers['?s_adwCompStringClause@CDXUTIMEEditBox@@1PAKA']=p.base+0x141928
new={'?r5ImeCandidateState@@3UR5ImeCandidateListState@@A':0x1427b0,'?r5ImeReadingString@@3PAGA':0x142530,'??0R5ImeCandidateListState@@QAE@XZ':0x85980,'??1R5ImeCandidateListState@@QAE@XZ':0x859a0,'_$E5':0xe42f0,'_$E6':0xe4380};providers.update({n:p.base+r for n,r in new.items()})
u='closure_menu_pool_process'
plan=[(u,'?Process@CMenuPool@@QAEXXZ',0x8200,342,'code'),(u,'$L95068',0xe01e0,21,'code'),(u,'$T95086',0xf79dc,36,'data'),(u,'??_C@_01CJBADKEH@?$IM?$AA@',0xe5950,2,'data'),(u,'??_C@_01OBMJLAEP@?$IE?$AA@',0xe5954,2,'data')]
providers.update({n:p.base+r for _,n,r,_,_ in plan})
providers['__ehhandler$?Process@CMenuPool@@QAEXXZ']=p.base+0xe01eb
providers['$T95081']=p.base+0xf79e4
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
 obj=COFF(d/(objunit+'.obj'))
 for sym in obj.symbols.values():
  name=sym['name']
  if name.startswith('__imp__') and name not in providers:
   api=name[7:].split('@')[0]
   info=imports[api];providers[name]=info['reference_va'];new_imports[name]=dict(info,library=info['dll'].replace('.dll','.lib'))
rows=[];union=set()
for u,n,r,z,kind in plan:
 o=COFF(d/(u+'.obj'));s=next(s for s in o.names[n] if s['section']>0);sec=o.sections[s['section']-1];assert s['value']==0 and sec['size']==z,(n,s,sec['size']);raw=bytearray(bytes(z) if sec['uninitialized'] else sec['bytes']);bs=[]
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
 rows.append(dict(unit=u,anchor=n,rva=r,size=z,section=s['section'],kind=kind,object_sha256=hashlib.sha256((d/(u+'.obj')).read_bytes()).hexdigest(),sha256=hashlib.sha256(raw).hexdigest(),whole_bytes_equal=True,bindings=bs))
a=accepted_code('evidence/checkpoint32/database-ime-acceptance.json')
assert len(union)==363 and len(union-a)==363
result=dict(status='SCOPED_FULL_PREFLIGHT_NOT_ACCEPTANCE',run=run,baseline=len(a),code_bytes=363,principal_code342=342,EH=21,data_bytes=40,new_unique_code=363,direct025_code_credit=0,regions=rows,new_imports=new_imports,excluded=['GetRakClient3 and WriteBYTE17 are inline/ambiguous or previously covered; no helper credit.'])
Path('build/agent-textdraw/menu-pool-process-reviewed-manifest.json').write_text(json.dumps(result,indent=2)+'\n')
Path('build/agent-textdraw/menu-pool-process-seeds.json').write_text(json.dumps([dict(unit=u,symbol=n,rva=r,size=z) for u,n,r,z,_ in plan],indent=2)+'\n')
print('PASS',len(rows),'whole sections',sum(len(r['bindings']) for r in rows),'bindings;363code/EH,40data')
