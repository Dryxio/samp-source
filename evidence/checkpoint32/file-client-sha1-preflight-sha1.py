import json,struct,sys,hashlib
from pathlib import Path
sys.path.insert(0,'tools');from binary import PE,COFF,u32,sha
from verify_actor_closure import initial_bytes
from draft_actor_contract import weak_aliases
run='cp32-sha1-trial1';unit='closure_vendor_sha1'
p=PE('private/samp.dll');o=COFF(Path('build')/run/(unit+'.obj'))
rows=json.load(open(Path('build')/run/'discovery-attached.json'))['rows'];by={r['symbol']:r for r in rows};aliases=weak_aliases(o)
roots={}
for r in rows:
 candidates=[c for c in r['candidates'] if 0x51600<=c['rva']<0x53800]
 assert len(candidates)==1,r['symbol']
 roots[r['symbol']]=candidates[0]['rva']
ids={};sections={};reasons={};queue=[]
def bind(n,va,reason):
 if n in ids:assert ids[n]==va,(n,hex(ids[n]),hex(va));return
 ids[n]=va;reasons[n]=reason;queue.append(n)
 if n in aliases:bind(aliases[n],va,'actual COFF weak alias '+n)
for n,rv in roots.items():bind(n,p.base+rv,'original principal identity and complete source semantics')
while queue:
 n=queue.pop();syms=[s for s in o.names.get(n,[]) if s['section']>0]
 if not syms:continue
 assert len(syms)==1,(n,syms)
 s=syms[0];si=s['section'];rv=ids[n]-p.base-s['value']
 if si in sections:assert sections[si]['rva']==rv;continue
 sec=o.sections[si-1];ref=initial_bytes(p,rv,sec['size']);raw=bytearray(bytes(sec['size']) if sec['uninitialized'] else sec['bytes']);bs=[]
 if sec['name']=='.text' and n in by:
  assert any(c['rva']==ids[n]-p.base for c in by[n]['candidates']),(n,hex(ids[n]))
 for j in range(sec['reloc_count']):
  off,idx,k=struct.unpack_from('<IIH',o.data,sec['reloc_ptr']+j*10);sym=o.symbols[idx]['name'];actual=u32(ref,off)
  if k==20:actual=(actual+p.base+rv+off+4)&0xffffffff
  else:assert k==6
  addend=u32(raw,off);bind(sym,(actual-addend)&0xffffffff,'actual whole-region relocation from '+n)
  assert ((rv+off) in p.relocations)==(k==6 and sym!='__except_list'),(n,sym,rv+off)
  struct.pack_into('<I',raw,off,u32(ref,off));bs.append(dict(symbol=sym,site_rva=rv+off,target_va=actual,kind=k))
 assert raw==ref,('FULL BODY MISMATCH',n,hex(rv),sec['size'])
 # No extra PE relocations are permitted, including ones absent in COFF.
 expected={rv+f['site_rva']-rv for f in bs if f['kind']==6 and f['symbol']!='__except_list'}
 assert {x for x in p.relocations if rv<=x<rv+sec['size']}==expected
 sections[si]=dict(unit=unit,section=si,rva=rv,size=sec['size'],anchor=n,kind=sec['name'],sha256=sha(ref),bindings=bs,whole_bytes_equal=True)
seed={}
for r in sections.values():
 if r['kind']=='.text':
  candidates=[s for s in o.symbols.values() if s and s['section']==r['section'] and s['type']==32 and s['storage']==2]
  if candidates:seed[candidates[0]['name']]=r['rva']+candidates[0]['value']
external={n:va for n,va in ids.items() if not any(s['section']>0 for s in o.names.get(n,[])) and n not in aliases}
old=json.load(open('config/checkpoint32/database-ime-contract.json'));owners={r['anchor']:p.base+r['rva']+r.get('anchor_offset',0) for r in old['regions']}
for n,va in external.items():
 if n in owners:assert owners[n]==va,(n,owners[n],va)
manifest=dict(status='WHOLE_REGION_PREFLIGHT_NOT_ACCEPTED_DEPENDENCIES_OPEN',run=run,roots=roots,seeds={unit:seed},regions=list(sections.values()),bindings_checked=sum(len(r['bindings']) for r in sections.values()),external_targets=external,identity_reasons=reasons,actual_weak_aliases={unit:aliases},notes='No source credit until every external true provider is closed and final linked gate passes.')
Path('build/sha1-reviewed-manifest.json').write_text(json.dumps(manifest,indent=2)+'\n')
print('ROOTS',len(roots),'SECTIONS',len(sections),'BINDINGS',manifest['bindings_checked'],'EXTERNALS',len(external))
for n,va in external.items():
 if n not in owners and n not in old['externals']:print('OPEN',n,hex(va))
