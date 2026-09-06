from pathlib import Path
import sys,json,struct,hashlib
sys.path.insert(0,'tools');from binary import PE,COFF,u32
from verify_checkpoint2 import map_symbols,relocs
from draft_actor_contract import weak_aliases
run=Path('build/cp32-nat-chat-mesh-linked1');unit='closure_vendor_sha1';o=COFF(run/(unit+'.obj'));p=PE(Path('private/samp.dll'));q=PE(run/'closure.dll');maps=map_symbols(run/'closure.map');manifest=json.loads(Path('build/sha1-reviewed-manifest.json').read_text());e='??_ECSHA1@@UAEPAXI@Z';g='??_GCSHA1@@UAEPAXI@Z';v='??_7CSHA1@@6B@'
assert weak_aliases(o)[e]==g;es=next(s for s in o.names[e] if s['storage']==105);assert es['section']==0;gs=next(s for s in o.names[g] if s['section']>0 and s['type']==32)
def addr(n):
 hits=maps[n];assert len(hits)==1,n;return hits[0][0]
assert addr(e)==addr(g) and maps[g][0][1].lower()==unit+'.obj'
code=next(r for r in manifest['regions'] if r.get('anchor')==g);table=next(r for r in manifest['regions'] if r.get('anchor')==v);assert code['size']==30 and table['size']==4
raw,fix=o.function(g);assert len(raw)==30;proof=[]
for pe,at,orig in [(p,p.base+code['rva'],True),(q,addr(g),False)]:
 patched=bytearray(raw)
 for f,b in zip(fix,code['bindings']):
  assert f['symbol']['name']==b['symbol'] and f['offset']==b['site_rva']-code['rva']
  target=b['target_va'] if orig else addr(f['symbol']['name']);off=f['offset'];value=target+u32(raw,off)-(at+off+4 if f['kind']==20 else 0);struct.pack_into('<I',patched,off,value&0xffffffff)
 assert bytes(patched)==pe.read(at-pe.base,30)
 assert {at-pe.base+f['offset'] for f in fix if f['kind']==6}=={r for r in pe.relocations if at-pe.base<=r<at-pe.base+30}
 proof.append(dict(image='original' if orig else 'linked',rva=at-pe.base,size=30,whole_bytes_equal=True,all_fixups_and_PE_relocations_equal=True))
ts=next(s for s in o.names[v] if s['section']>0);sec=o.sections[ts['section']-1];tf=relocs(o,ts['section']);assert sec['size']==4 and len(tf)==1 and tf[0]['offset']==0 and tf[0]['kind']==6 and tf[0]['symbol']['name']==e and u32(sec['bytes'],0)==0
assert u32(p.read(table['rva'],4),0)==p.base+code['rva'] and table['rva'] in p.relocations
assert q.read(addr(v)-q.base,4)==struct.pack('<I',addr(g)) and addr(v)-q.base in q.relocations
x=dict(status='ACTUAL_SHA1_WEAK_DELETING_ALIAS_FULL_VFT_AND_CODE_QUALIFIED',run=str(run),object_sha256=hashlib.sha256((run/(unit+'.obj')).read_bytes()).hexdigest(),dll_sha256=hashlib.sha256(q.data).hexdigest(),map_sha256=hashlib.sha256((run/'closure.map').read_bytes()).hexdigest(),alias=dict(source=e,canonical=g,source_storage=105,source_undefined=True,real_aux_fallback_verified=True,alias_va=addr(e),canonical_va=addr(g),owner=maps[g][0][1]),code=proof,vtable=dict(symbol=v,source_section=ts['section'],size=4,original_rva=table['rva'],linked_rva=addr(v)-q.base,slot0_references_actual_weak_alias=True,full_bytes_and_relocations_equal=True),extra_credit=0)
Path('build/nat-chat-mesh-sha1-deleting-alias-qualified.json').write_text(json.dumps(x,indent=2)+'\n');print('PASS real SHA1 E->G, full VFT4 +G30 both images')
