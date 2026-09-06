import sys,json,struct,hashlib
from pathlib import Path
sys.path.insert(0,'tools')
from binary import COFF,PE
from draft_actor_contract import weak_aliases
from rank_uncovered_candidates import accepted_code
m=json.loads(Path('build/mesh-reviewed-manifest.json').read_text());p=PE(Path('private/samp.dll'));run=Path(m['run']);u='closure_vendor_fully_connected_mesh';o=COFF(run/(u+'.obj'));c=json.loads(Path('config/checkpoint32/directory-ime-contract.json').read_text())
providers={r['anchor']:p.base+r['rva']-r.get('anchor_offset',0) for r in c['regions']};providers.update({n:x['reference_va'] for n,x in c['externals'].items()})
for r in m['regions']:
 for sym in o.symbols.values():
  if sym['section']==r['section']:providers[sym['name']]=p.base+r['rva']+sym['value']
wa=weak_aliases(o);assert wa=={'??_EFullyConnectedMesh@@UAEPAXI@Z':'??_GFullyConnectedMesh@@UAEPAXI@Z'},wa
for n,t in wa.items():providers[n]=providers[t]
rows=[];union=set()
for r in m['regions']:
 sec=o.sections[r['section']-1];assert sec['size']==r['size'];raw=bytearray(sec['bytes']);assert len(r['bindings'])==sec['reloc_count'];bs=[];ab=set()
 for i,f in enumerate(r['bindings']):
  off,si,k=struct.unpack_from('<IIH',o.data,sec['reloc_ptr']+10*i);sym=o.symbols[si]['name'];assert f['symbol'].split('::')[-1]==sym;v=providers[sym];add=struct.unpack_from('<I',raw,off)[0];target=(v+add)&0xffffffff;site=r['rva']+off;assert target==f['target_va']
  struct.pack_into('<I',raw,off,(target-(p.base+site+4 if k==20 else 0))&0xffffffff)
  if k==6 and sym!='__except_list':ab.add(site)
  bs.append(dict(symbol=sym,symbol_va=v,addend=add,target_va=target,site_rva=site,kind=k))
 assert raw==p.read(r['rva'],r['size']);assert hashlib.sha256(raw).hexdigest()==r['sha256'];assert ab=={x for x in p.relocations if r['rva']<=x<r['rva']+r['size']}
 if sec['flags']&0x20:union.update(range(r['rva'],r['rva']+r['size']))
 rows.append(dict(r,bindings=bs))
a=accepted_code('evidence/checkpoint32/directory-ime-acceptance.json');assert len(a)==318918;assert len(union-a)==630
vft=[struct.unpack('<I',p.read(0xe6dbc+4*i,4))[0] for i in range(11)];assert vft[4]==p.base+0x2e630 and vft[10]==p.base+0x2e610
other=struct.unpack('<I',p.read(0x2ec34,4))[0];assert other==p.base+0xe6de8 and other!=p.base+0xe6dbc
out=dict(status='INDEPENDENT_FULL_OBJECT_PREFLIGHT_LINK_ALIAS_PENDING',run=str(run),baseline=len(a),new_unique_code=630,code_including_EH=630,data_bytes=80,sections=10,bindings=sum(len(r['bindings']) for r in rows),unowned_dependencies=[],actual_weak_aliases=wa,rejected_ctor2EC30=hex(other),vft_slots=[hex(v) for v in vft],regions=rows)
Path('build/agent-textdraw/mesh-independent-review.json').write_text(json.dumps(out,indent=2)+'\n');print({k:v for k,v in out.items() if k!='regions'})
