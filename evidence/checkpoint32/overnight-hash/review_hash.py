import json,sys,struct
from pathlib import Path
sys.path.insert(0,'tools')
from binary import COFF,PE,sha,u32
from verify_checkpoint2 import relocs
from verify_actor_closure import initial_bytes
from rank_uncovered_candidates import accepted_code
from draft_actor_contract import weak_aliases
RUN=Path('build/overnight-hash-two-objects');BASE='overnight-socket-one'
units=['closure_vendor_md5','closure_vendor_fhicl_sha1'];objs={u:COFF(RUN/(u+'.obj')) for u in units};pe=PE('private/samp.dll')
dis=json.load(open('build/overnight-hash-two-objects/discovery-attached.json'))['rows'];known={};roots=[]
for r in dis:
 if len(r['candidates'])==1:
  known[(r['unit'],r['symbol'])]=r['candidates'][0]['rva']
  sym=next(s for s in objs[r['unit']].names[r['symbol']] if s['section']>0)
  if sym['storage']==2:roots.append(dict(unit=r['unit'],symbol=r['symbol'],rva=r['candidates'][0]['rva'],size=r['size']))
# An unused no-op is never selected only from its identical RET byte.

# Finish is not reached by emitted calls; ambiguous no-op stays outside coverage.
base=json.load(open('config/checkpoint32/'+BASE+'-contract.json'));providers={r['anchor']:pe.base+r['rva']+r.get('anchor_offset',0) for r in base['regions']};providers.update({n:r['reference_va'] for n,r in base['externals'].items()})

loc={};anchors={};todo=[];regions=[]
def select(u,s,rva):
 key=(u,s['section']);sec=objs[u].sections[s['section']-1];start=rva-s['value']
 if sec['flags']&0x20:
  candidates=next(r['candidates'] for r in dis if r['unit']==u and r['symbol']==s['name'])
  assert rva in [c['rva'] for c in candidates],('No complete source fingerprint at actual call target',u,s['name'],hex(rva))
 if key in loc:assert loc[key]==start
 else:loc[key]=start;anchors[key]=s['name'];todo.append(key)
for r in roots:select(r['unit'],next(s for s in objs[r['unit']].names[r['symbol']] if s['section']>0),r['rva'])
while todo:
 u,index=todo.pop(0);o=objs[u];sec=o.sections[index-1];rva=loc[(u,index)];raw=bytearray(bytes(sec['size']) if sec['uninitialized'] else sec['bytes']);ref=initial_bytes(pe,rva,len(raw));bindings=[]
 for f in relocs(o,index):
  s=f['symbol'];n=s['name'];off=f['offset'];kind=f['kind'];assert kind in (6,20);add=u32(raw,off);target=u32(ref,off)
  if kind==20:target=(target+pe.base+rva+off+4)&0xffffffff
  dest=(target-add)&0xffffffff
  if s['section']>0:select(u,s,dest-pe.base)
  else:
   defs=[(v,x) for v,obj in objs.items() for x in obj.names.get(n,[]) if x['section']>0 and x['storage']==2]
   if defs:
    assert len(defs)==1;v,x=defs[0];select(v,x,dest-pe.base)
   else:assert providers.get(n)==dest,('Unreviewed external',n,hex(dest))
  value=target if kind==6 else target-pe.base-rva-off-4
  struct.pack_into('<I',raw,off,value&0xffffffff);bindings.append(dict(symbol=n,site_rva=rva+off,kind=kind,target_va=target))
 assert bytes(raw)==ref,('Full bytes differ',u,index,hex(rva))
 assert {a for a in pe.relocations if rva<=a<rva+len(raw)}=={b['site_rva'] for b in bindings if b['kind']==6}
 a=anchors[(u,index)];s=next(s for s in o.names[a] if s['section']==index)
 regions.append(dict(unit=u,section=index,anchor=a,anchor_offset=s['value'],rva=rva,size=len(raw),kind='code' if sec['flags']&0x20 else 'zero' if sec['uninitialized'] else 'data',sha256=sha(ref),bindings=bindings))
covered=accepted_code('evidence/checkpoint32/'+BASE+'-acceptance.json');code=set().union(*(set(range(r['rva'],r['rva']+r['size'])) for r in regions if r['kind']=='code'))
result=dict(status='PASS_FULL_SOURCE_REGIONS_ALL_COFF_PE_TARGETS',baseline=BASE,run=str(RUN),roots=roots,regions=regions,new_unique_code=len(code-covered),object_sha256={u:sha((RUN/(u+'.obj')).read_bytes()) for u in units},source_sha256={u:sha((RUN/'client/saco'/(u+'.cpp')).read_bytes()) for u in units},actual_weak_aliases={u:weak_aliases(objs[u]) for u in units},provenance='Existing source MD5 Colin Plumb 1993 public domain and fhicl SHA1 Paul E. Jones 1998/2009, source/header amalgamations unchanged. Unique entire function fingerprints anchor selected methods; actual original call targets and complete COFF/PE relocations independently checked. Ambiguous constructors, destructors and wrappers excluded. No byte copying, truncation or external implementation bypass.')
Path('evidence/checkpoint32/overnight-hash').mkdir(exist_ok=True)
Path('evidence/checkpoint32/overnight-hash/hash-two-reviewed-manifest.json').write_text(json.dumps(result,indent=2)+'\n')
print('PASS',len(regions),'regions',len(code-covered),'new code')
