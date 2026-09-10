import sys,json,struct,hashlib
from pathlib import Path
sys.path[:0]=['tools','build/overnight']
from binary import COFF,PE,sha,u32
from verify_actor_closure import initial_bytes
from verify_checkpoint2 import relocs
from external_support import extend
from integration_pipeline import read,write,pinned
p=read(sys.argv[1]);extra=read(pinned(p['reviewed_externals']));base=extend(read(pinned(p['base']['contract'])),extra);pe=PE('private/samp.dll')
providers={n:r['reference_va'] for n,r in base['externals'].items()};providers.update({r['anchor']:pe.base+r['rva']+r.get('anchor_offset',0) for r in base['regions']})
for item in p['reviews']:
 for r in read(pinned(item))['regions']:
  if r['anchor'] not in providers:providers[r['anchor']]=pe.base+r['rva']+r.get('anchor_offset',0)
for item in p['reviews']:
 m=read(pinned(item))
 if m['status']=='PASS_FULL_SOURCE_REGIONS_ALL_COFF_PE_TARGETS':continue
 assert m['status'].startswith('PASS_FULL_SOURCE_REGIONS_ALL_COFF_PE_TARGETS_PENDING_') and not m.get('errors')
 run=Path(m['run']);run=run if run.parts[0]=='build' else Path('build')/run
 objs={u:COFF(run/(u+'.obj')) for u in m['object_sha256']}
 for u,o in objs.items():assert sha(o.data)==m['object_sha256'][u]
 for r in m['regions']:
  o=objs[r['unit']];sec=o.sections[r['section']-1];assert sec['size']==r['size'];raw=bytearray(bytes(sec['size']) if sec['uninitialized'] else sec['bytes']);ref=initial_bytes(pe,r['rva'],r['size']);fixes=relocs(o,r['section'])
  assert len(fixes)==len(r['bindings'])
  for f,b in zip(fixes,r['bindings']):
   off=f['offset'];kind=f['kind'];n=f['symbol']['name'];assert (off,kind,n)==(b['site_rva']-r['rva'],b['kind'],b['symbol'])
   if f['symbol']['section']<=0:assert providers[n]==(b['target_va']-u32(raw,off))&0xffffffff,(n,b)
   val=b['target_va']-(pe.base+r['rva']+off+4 if kind==20 else 0);struct.pack_into('<I',raw,off,val&0xffffffff)
  assert bytes(raw)==ref and sha(ref)==r['sha256']
  assert {a for a in pe.relocations if r['rva']<=a<r['rva']+r['size']}=={b['site_rva'] for b in r['bindings'] if b['kind']==6}
 m['prior_status']=m['status'];m['status']='PASS_FULL_SOURCE_REGIONS_ALL_COFF_PE_TARGETS';m['integrator_review']='Original exact source bytes and complete named COFF/PE targets reverified; new imports independently bound to original PE descriptors with pinned library provenance. Actual linked archive providers remain mandatory through external_pipeline and unchanged final gate.'
 dest=Path('evidence/checkpoint32/overnight-wrappers')/Path(item['path']).name;write(dest,m);item.update(path=str(dest),sha256=sha(dest.read_bytes()))
for k in p['qualification_inputs']:
 if k['path'].startswith('build/overnight/') and Path(k['path']).exists():k['sha256']=sha(Path(k['path']).read_bytes())
write(sys.argv[2],p)
print('Source regions and new original import identities verified')
