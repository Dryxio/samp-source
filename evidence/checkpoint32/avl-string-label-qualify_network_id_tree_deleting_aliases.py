from pathlib import Path
import sys,json,struct,hashlib,re,argparse
sys.path.insert(0,'tools');from binary import PE,COFF,u32
from draft_actor_contract import weak_aliases
ap=argparse.ArgumentParser();ap.add_argument('--run',required=True);args=ap.parse_args();run=Path('build')/args.run;p=PE(Path('private/samp.dll'));q=PE(run/'closure.dll');mp=(run/'closure.map').read_text()
def mapped(n):
 m=re.search(r'^\s*[0-9A-Fa-f]{4}:[0-9A-Fa-f]{8}\s+'+re.escape(n)+r'\s+([0-9A-Fa-f]{8})([^\r\n]*)',mp,re.M);assert m,n
 return dict(symbol=n,va=int(m.group(1),16),owner=m.group(2).strip())
rows=[]
for cls,unit,rva,table in [('?$BinarySearchTree@UNetworkIDNode@@@DataStructures','closure_vendor_network_id_tree',0x332d0,0xe7180),('?$AVLBalancedBinarySearchTree@UNetworkIDNode@@@DataStructures','closure_vendor_network_id_tree',0x333c0,0xe7184)]:
 c=COFF(run/(unit+'.obj'));e='??_E'+cls+'@@UAEPAXI@Z';g='??_G'+cls+'@@UAEPAXI@Z';vt='??_7'+cls+'@@6B@';assert weak_aliases(c)[e]==g
 es=next(s for s in c.names[e] if s['storage']==105);assert es['section']==0;gs=next(s for s in c.names[g] if s['section']>0);assert gs['type']==32
 em,gm,tm=mapped(e),mapped(g),mapped(vt);assert em['va']==gm['va'];assert unit in gm['owner']
 raw,fixes=c.function(g);assert len(raw)==30;data=bytearray(raw);binds=[]
 for f in fixes:
  at=f['offset'];entry=mapped(f['symbol']['name']);value=entry['va']+u32(data,at)
  if f['kind']==20:value-=gm['va']+at+4
  else:assert f['kind']==6
  struct.pack_into('<I',data,at,value&0xffffffff);binds.append(dict(symbol=f['symbol']['name'],offset=at,kind=f['kind'],map_provider=entry))
 assert bytes(data)==q.read(gm['va']-q.base,30)
 originalslot=table+0;linkedslot=tm['va']-q.base+0;assert u32(p.read(originalslot,4),0)==p.base+rva;assert u32(q.read(linkedslot,4),0)==gm['va'];assert originalslot in p.relocations and linkedslot in q.relocations
 ts=next(s for s in c.names[vt] if s['section']>0);sec=c.sections[ts['section']-1];rel=next((o,i,k) for o,i,k in [struct.unpack_from('<IIH',c.data,sec['reloc_ptr']+j*10) for j in range(sec['reloc_count'])] if o==0);assert rel[2]==6 and c.symbols[rel[1]]['name']==e
 rows.append(dict(unit=unit,alias=e,canonical=g,source_alias_storage=105,source_alias_undefined=True,weak_aux_resolves_canonical=True,canonical_defined_section=gs['section'],alias_map=em,canonical_map=gm,whole_linked_body30_compared=True,bindings=binds,selected_vtable_fixup=dict(section=ts['section'],offset=0,kind=6,alias=e,original_slot_rva=originalslot,linked_slot_rva=linkedslot,actual_provider_va=gm['va']),object_sha256=hashlib.sha256((run/(unit+'.obj')).read_bytes()).hexdigest(),extra_credit=0))
a=dict(status='ACTUAL_COMPILER_WEAK_ALIASES_AND_SELECTED_VFT_FIXUPS_QUALIFIED',run=str(run),dll_sha256=hashlib.sha256((run/'closure.dll').read_bytes()).hexdigest(),map_sha256=hashlib.sha256((run/'closure.map').read_bytes()).hexdigest(),aliases=rows,note='Two genuine storage105 weak externals from complete 4-byte VFTs resolve to their real complete deleting destructors; source preflight already verified original identities and all targets.',extra_credit=0)
f=Path('build/agent-independent')/('network-id-tree-deleting-aliases-'+Path(args.run).name+'.json');f.write_text(json.dumps(a,indent=2)+'\n');print(f);print('PASS two storage105 weak aliases, canonical MAP addresses and selected full VFT fixups')
