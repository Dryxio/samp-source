from pathlib import Path
import argparse,sys,json,struct,hashlib
sys.path.insert(0,'tools')
from binary import PE,COFF
ap=argparse.ArgumentParser();ap.add_argument('--run',required=True);args=ap.parse_args()
p=PE(Path('private/samp.dll'));c=json.loads(Path('config/checkpoint32/tcp-font-contract.json').read_text());d=Path('build')/args.run;o=COFF(d/'closure_util.obj')
providers={r['anchor']:p.base+r['rva']-r.get('anchor_offset',0) for r in c['regions']+c['pending']};providers.update({n:v['reference_va'] for n,v in c['externals'].items()})
providers['_strtoul']=p.base+0xc7df5
parser='?GetColorFromEmbedCode@@YAKPAD@Z';old=[r for r in c['regions'] if r['unit']=='closure_util'];plan=[dict(unit='closure_util',anchor=parser,rva=0xb5e80,size=271,kind='code')]+old
rows=[]
for r in plan:
 n=r['anchor'];ss=[s for s in o.names.get(n,[]) if s['section']>0]
 assert len(ss)==1,(n,ss)
 s=ss[0];sec=o.sections[s['section']-1];offset=r.get('offset',0);z=r['size'];rv=r['rva']
 assert sec['size']==z and offset==0,(n,'whole section',sec['size'],z,offset)
 raw=bytearray(bytes(z) if sec['uninitialized'] else sec['bytes']);bindings=[]
 for i in range(sec['reloc_count']):
  off,si,k=struct.unpack_from('<IIH',o.data,sec['reloc_ptr']+10*i);sym=o.symbols[si];name=sym['name'];a=struct.unpack_from('<I',raw,off)[0]
  sv=(p.base+rv+sym['value'] if sym['section']==s['section'] else providers[name]);target=(sv+a)&0xffffffff;orig=struct.unpack('<I',p.read(rv+off,4))[0]
  assert target==(orig if k==6 else (orig+p.base+rv+off+4)&0xffffffff),(n,name,hex(target),hex(orig))
  struct.pack_into('<I',raw,off,(target-(p.base+rv+off+4 if k==20 else 0))&0xffffffff);bindings.append(dict(symbol=name,site_rva=rv+off,symbol_va=sv,addend=a,target_va=target,kind=k))
 if sec['uninitialized']:
  ps=next(s for s in p.sections if s['rva']<=rv<s['rva']+s['virtual_size']);assert rv>=ps['rva']+ps['size'] and rv+z<=ps['rva']+ps['virtual_size'];original=bytes(z)
 else:original=p.read(rv,z)
 assert raw==original,(n,'bytes')
 assert {b['site_rva'] for b in bindings if b['kind']==6 and b['symbol']!='__except_list'}=={a for a in p.relocations if rv<=a<rv+z},(n,'PE')
 rows.append(dict(unit='closure_util',anchor=n,rva=rv,size=z,section=s['section'],kind=r['kind'],sha256=hashlib.sha256(raw).hexdigest(),bindings=bindings))
objsha=hashlib.sha256((d/'closure_util.obj').read_bytes()).hexdigest()
for row in rows:row['object_sha256']=objsha
out=dict(status='COMPLETE_PARSER_AND_ALL_ACCEPTED_UTIL_SIBLINGS_PASS',run=args.run,new_unique_code=271,EH_bytes=0,direct025_code_credit=0,parser=rows[0],accepted_sibling_count=len(old),accepted_siblings=rows[1:],new_CRT_provider={'_strtoul':dict(reference_va=p.base+0xc7df5,expected_library_owner='LIBCMT:strtol.obj',actual_archive_and_final_link_qualification_pending=True)},source_change='Only first IsHexChar(char) predicate expanded in normal C++; original executes five following callsB5E10. Buffer17, strncpy6, strtoul16 and all other owner definitions unchanged.')
Path('build/agent-textdraw/font-ansi-final-reviewed-manifest.json').write_text(json.dumps(out,indent=2)+'\n')
Path('build/agent-textdraw/font-ansi-seeds.json').write_text(json.dumps([dict(unit='closure_util',symbol=parser,rva=0xb5e80,size=271)],indent=2)+'\n')
print('PASS271 new;',len(old),'accepted whole sibling sections revalidated;',len(rows[0]['bindings']),'parser bindings')
