import sys,json,struct
from pathlib import Path
sys.path.insert(0,'tools')
from binary import COFF,PE,sha,u32
from verify_checkpoint2 import relocs
from verify_actor_closure import initial_bytes
from rank_uncovered_candidates import accepted_code
run=Path(sys.argv[1] if len(sys.argv)>1 else 'build/cp32-trial30d-other-thunks1');unit='closure_rakclient_interface_adjustors';o=COFF(run/(unit+'.obj'));compiler=COFF('build/cp32-trial30d-thunks1/eval_vendor_rakclient_ctor_thunks.obj');p=PE('private/samp.dll');c=json.load(open('config/checkpoint32/trial30d-one-contract.json'));prov={r['anchor']:p.base+r['rva']+r.get('anchor_offset',0) for r in c['regions']};covered=accepted_code('evidence/checkpoint32/trial30d-one-acceptance.json');rows=[];seeds=[];audit=[]
for x in json.load(open('build/trial30d/other-thunk-candidates.json')):
 method=x['target'].split('@')[0];s=next(s for s in o.symbols.values() if s['section']>0 and s['name'].startswith(method+'@R5RakClientInterfaceAdjustors@@'));sec=o.sections[s['section']-1];fs=relocs(o,s['section']);assert sec['size']==11 and len(fs)==1;f=fs[0];assert f['offset']==7 and f['kind']==20 and f['symbol']['name']==x['target'];target=prov[x['target']];r=x['rva'];assert u32(initial_bytes(p,0xe75c8+4*x['slot'],4),0)==p.base+r
 cs=next(s for s in compiler.names[x['compiler_symbol']] if s['section']>0);csec=compiler.sections[cs['section']-1];cf=relocs(compiler,cs['section']);assert sec['bytes']==csec['bytes'] and [(a['offset'],a['kind'],a['symbol']['name']) for a in fs]==[(a['offset'],a['kind'],a['symbol']['name']) for a in cf]
 raw=bytearray(sec['bytes']);assert u32(raw,7)==0;struct.pack_into('<I',raw,7,(target-(p.base+r+11))&0xffffffff);ref=initial_bytes(p,r,11);assert bytes(raw)==ref;assert not any(r<=z<r+11 for z in p.relocations)
 rows.append(dict(unit=unit,section=s['section'],anchor=s['name'],anchor_offset=0,rva=r,size=11,kind='code',sha256=sha(ref),bindings=[dict(symbol=x['target'],site_rva=r+7,kind=20,target_va=target)]));seeds.append(dict(unit=unit,symbol=s['name'],rva=r,size=11));audit.append(dict(**x,compiled_symbol=s['name'],independent_compiler_section=cs['section'],actual_target=target))
new={a for r in rows for a in range(r['rva'],r['rva']+r['size'])}-covered;assert len(new)==165
Path('build/trial30d/other-adjustors-reviewed-manifest.json').write_text(json.dumps(dict(status='PASS_COMPLETE_SYMBOLIC_ADAPTERS_INDEPENDENT_COMPILER_ABI',run=run.name,new_unique_code=len(new),regions=rows,independent_compiler_audit=audit,source_sha256=sha((run/('client/saco/'+unit+'.cpp')).read_bytes()),object_sha256=sha((run/(unit+'.obj')).read_bytes()),scope='15 whole symbolic this-adjustors for accepted real callees; no constructor/vtable/unimplemented callee credit.'),indent=2))
Path('build/trial30d/other-adjustors-seeds.json').write_text(json.dumps(seeds,indent=2));print('PASS',len(new),'new unique bytes, all15 adapters compiler/COFF/PE/vtable/callee verified')
