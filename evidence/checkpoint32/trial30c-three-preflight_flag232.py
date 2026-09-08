import sys,json,struct
from pathlib import Path
import hashlib
sys.path.insert(0,'tools');from binary import PE,COFF,sha,u32
from verify_checkpoint2 import relocs
from verify_actor_closure import initial_bytes
from rank_uncovered_candidates import accepted_code
p=PE('private/samp.dll');c=json.load(open('config/checkpoint32/trial30b-two-contract.json'));run=Path('build/cp32-trial30c-final-probe');objs={u:COFF(run/(u+'.obj')) for u in ['closure_rpc_net_flag232']}
providers={r['anchor']:p.base+r['rva']+r.get('anchor_offset',0) for r in c['regions']};providers.update({n:e['reference_va'] for n,e in c['externals'].items()});providers['__except_list']=0
plan=[('closure_rpc_net_flag232','?SetNetGameFlag232RPCTransfer@@YAXPAURPCParameters@@@Z',0x17de0,95)]
todo=[];seeds=[];rows=[];seen={}
for u,n,r,z in plan:
 providers[n]=p.base+r;s=next(x for x in objs[u].names[n] if x['section']>0);assert objs[u].sections[s['section']-1]['size']==z,(n,objs[u].sections[s['section']-1]['size'],z);todo.append((u,s['section'],r));seeds.append(dict(unit=u,symbol=n,rva=r,size=z))
while todo:
 u,si,r=todo.pop();k=(u,si)
 if k in seen:assert seen[k]==r;continue
 seen[k]=r;o=objs[u];sec=o.sections[si-1];raw=bytearray(sec['bytes'] if sec['bytes'] else bytes(sec['size']));ref=initial_bytes(p,r,sec['size']);bindings=[]
 for f in relocs(o,si):
  off=f['offset'];s=f['symbol'];a=u32(raw,off);target=u32(ref,off)
  if f['kind']==20:target=(target+p.base+r+off+4)&0xffffffff
  va=(target-a)&0xffffffff
  if s['section']>0:todo.append((u,s['section'],va-p.base-s['value']))
  else:assert providers[s['name']]==va,(u,s['name'],hex(va),hex(providers.get(s['name'],0)))
  value=(target-(p.base+r+off+4 if f['kind']==20 else 0))&0xffffffff;struct.pack_into('<I',raw,off,value);bindings.append(dict(symbol=s['name'],site_rva=r+off,kind=f['kind'],target_va=target))
 assert bytes(raw)==ref,(u,hex(r))
 assert {x for x in p.relocations if r<=x<r+sec['size']}=={b['site_rva'] for b in bindings if b['kind']==6 and b['symbol']!='__except_list'}
 anchor=next(x for x in o.symbols.values() if x['section']==si and x['name']!=sec['name']);rows.append(dict(unit=u,section=si,anchor=anchor['name'],anchor_offset=anchor['value'],rva=r,size=sec['size'],kind='code' if sec['flags']&0x20 else 'data',sha256=sha(ref),bindings=bindings))
covered=accepted_code('evidence/checkpoint32/trial30b-two-acceptance.json');new={a for r in rows if r['kind']=='code' for a in range(r['rva'],r['rva']+r['size'])}-covered;assert len(new)==95
out=dict(status='PASS_FULL_SOURCE_REGIONS_ALL_COFF_PE_TARGETS',run=run.name,baseline=len(covered),new_unique_code=len(new),regions=rows,source025_direct=0,object_sha256={u:sha((run/(u+'.obj')).read_bytes()) for u in objs},source_sha256={u:sha((run/('client/saco/'+u+'.cpp')).read_bytes()) for u in objs},typed_layout_note='Existing CNetGame owned byte field232, bool read1bit via acceptedBitStream; no calls through unknownvirtualtarget and no new stores or CRT init.')
Path('build/trial30c/client-agent/net-flag232-reviewed-manifest.json').write_text(json.dumps(out,indent=2)+'\n');Path('build/trial30c/client-agent/net-flag232-seeds.json').write_text(json.dumps(seeds,indent=2)+'\n');print('PASS',len(rows),'whole regions',len(new),'unique',sum(len(r['bindings']) for r in rows),'fixups')
