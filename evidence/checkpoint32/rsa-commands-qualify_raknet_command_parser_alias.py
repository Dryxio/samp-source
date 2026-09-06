from pathlib import Path
import sys,json,struct,hashlib,argparse
sys.path.insert(0,'tools')
from binary import PE,COFF,u32
from verify_checkpoint2 import map_symbols,relocs
from draft_actor_contract import weak_aliases
ap=argparse.ArgumentParser();ap.add_argument('--run',required=True);args=ap.parse_args()
run=Path('build')/args.run;p=PE(Path('private/samp.dll'));q=PE(run/'closure.dll');maps=map_symbols(run/'closure.map');m=json.loads(Path('build/agent-independent/raknet-command-parser-reviewed-manifest.json').read_text())
def addr(n):
 hits=maps[n];assert len(hits)==1,(n,hits);return hits[0][0]
def compare_section(si,n):
 sec=o.sections[si-1];r=regions[si];assert sec['size']==r['size'];fs=relocs(o,si);bindings={b['site_rva']-r['rva']:b for b in r['bindings']};proof=[]
 sy=next(s for s in o.names[n] if s['section']==si)
 for pe,at,orig in [(p,p.base+r['rva'],True),(q,addr(n)-sy['value'],False)]:
  raw=bytearray(sec['bytes']);pes=set()
  for f in fs:
   off=f['offset'];b=bindings[off];assert b['symbol']==f['symbol']['name'] and b['kind']==f['kind'];target=b['target_va'] if orig else addr(f['symbol']['name']);v=target+u32(sec['bytes'],off)-(at+off+4 if f['kind']==20 else 0);struct.pack_into('<I',raw,off,v&0xffffffff)
   if f['kind']==6:pes.add(at-pe.base+off)
  assert bytes(raw)==pe.read(at-pe.base,len(raw)),(n,'whole bytes')
  assert pes=={v for v in pe.relocations if at-pe.base<=v<at-pe.base+len(raw)},(n,'PE fixups')
  proof.append(dict(image='original' if orig else 'linked',rva=at-pe.base,size=len(raw),sha256=hashlib.sha256(raw).hexdigest(),whole_bytes_equal=True,all_fixups_and_PE_relocations_equal=True))
 return proof
out=[];object_hashes={}
for unit in ['eval_vendor_raknet_command_parser']:
 o=COFF(run/(unit+'.obj'));object_hashes[unit]=hashlib.sha256(o.data).hexdigest();regions={r['section']:r for r in m['regions'] if r['unit']==unit}
 for e,g in weak_aliases(o).items():
  assert e.startswith('??_E');es=next(s for s in o.names[e] if s['storage']==105);assert es['section']==0
  gs=next(s for s in o.names[g] if s['section']>0 and s['type']==32);assert addr(e)==addr(g) and maps[g][0][1].lower()==unit+'.obj'
  v='??_7'+g[len('??_G'):].split('@@UAEPAXI@Z')[0]+'@@6B@'
  vs=next(s for s in o.names[v] if s['section']>0);sec=o.sections[vs['section']-1];fx=relocs(o,vs['section']);assert any(f['offset']==(40 if unit=='eval_vendor_packet_console_logger' else 0) and f['kind']==6 and f['symbol']['name']==e for f in fx)
  assert sec['size']==(68 if unit=='eval_vendor_packet_console_logger' else 60),(v,sec['size'])
  out.append(dict(weak=e,canonical=g,table=v,source_storage=105,source_undefined=True,actual_aux_fallback=True,canonical_va=addr(g),alias_va=addr(e),owner=maps[g][0][1],full_code=compare_section(gs['section'],g),full_vft=compare_section(vs['section'],v)))
assert len(out)==1
result=dict(status='PASS_TRUE_RAKNET_COMMAND_PARSER_DELETING_ALIAS',run=str(run),object_sha256=object_hashes,dll_sha256=hashlib.sha256(q.data).hexdigest(),map_sha256=hashlib.sha256((run/'closure.map').read_bytes()).hexdigest(),aliases=out,extra_code_credit=0)
Path('build/agent-independent/raknet-command-parser-alias-qualified.json').write_text(json.dumps(result,indent=2)+'\n');print('PASS',len(out),'complete code and VFT both images')
