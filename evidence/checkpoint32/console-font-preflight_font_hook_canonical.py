from pathlib import Path
import sys,json,struct,hashlib
sys.path.insert(0,'tools')
from binary import PE,COFF
from rank_uncovered_candidates import accepted_code
p=PE(Path('private/samp.dll'));run='cp32-console-font-ob1-v3';d=Path('build')/run
c=json.loads(Path('config/checkpoint32/tcp-font-contract.json').read_text());providers={r['anchor']:p.base+r['rva']-r.get('anchor_offset',0) for r in c['regions']};providers.update({n:v['reference_va'] for n,v in c['externals'].items()})
proxy='closure_font_sprite_proxy';font='d3dhook_ID3DXFontHook';po=COFF(d/(proxy+'.obj'))
plan=[]
names=['QueryInterface','AddRef','Release','GetDevice','GetTransform','SetTransform','SetWorldViewRH','SetWorldViewLH','Begin','Flush','End','OnLostDevice','OnResetDevice']
for i,name in enumerate(names):
 n=next(n for n in po.names if n.startswith('?'+name+'@R5FontSpriteProxy@@'))
 plan.append((proxy,n,0x73de0+16*i,13 if i==0 else 14,'code'))
plan.extend([(proxy,next(n for n in po.names if n.startswith('?Draw@R5FontSpriteProxy@@')),0x73bc0,29,'code'),(font,'?DrawTextA@ID3DXFontHook@@UAGHPAUID3DXSprite@@PBDHPAUtagRECT@@KK@Z',0x73be0,229,'code'),(proxy,'??_7R5FontSpriteProxy@@6B@',0xea744,56,'data'),(proxy,'?r5FontSpriteProxy@@3VR5FontSpriteProxy@@A',0x102bf0,4,'data'),('closure_store_font_actual_sprite','?r5FontActualSprite@@3PAUID3DXSprite@@A',0x136ea0,4,'zero'),('closure_store_font_stripped_text','?r5FontStrippedText@@3PADA',0x140af0,4,'zero')])
providers.update({n:p.base+r for _,n,r,_,_ in plan})
assert not any(s['name'].startswith('.CRT') for s in po.sections)
rows=[];code=set()
for u,n,r,z,kind in plan:
 op=d/(u+'.obj');o=COFF(op);s=next(x for x in o.names[n] if x['section']>0);sec=o.sections[s['section']-1];assert s['value']==0 and sec['size']==z,(n,sec['size']);raw=bytearray(bytes(z) if sec['uninitialized'] else sec['bytes']);bindings=[]
 for i in range(sec['reloc_count']):
  off,si,k=struct.unpack_from('<IIH',o.data,sec['reloc_ptr']+10*i);name=o.symbols[si]['name'];a=struct.unpack_from('<I',raw,off)[0];sv=providers[name];target=(sv+a)&0xffffffff;original=struct.unpack('<I',p.read(r+off,4))[0];assert target==(original if k==6 else (original+p.base+r+off+4)&0xffffffff),(n,name)
  struct.pack_into('<I',raw,off,(target-(p.base+r+off+4 if k==20 else 0))&0xffffffff);bindings.append(dict(symbol=name,site_rva=r+off,symbol_va=sv,addend=a,target_va=target,kind=k))
 if sec['uninitialized']:
  ps=next(s for s in p.sections if s['rva']<=r<s['rva']+s['virtual_size']);assert r>=ps['rva']+ps['size'] and r+z<=ps['rva']+ps['virtual_size'];original=bytes(z)
 else:original=p.read(r,z)
 assert raw==original,n
 assert {b['site_rva'] for b in bindings if b['kind']==6}=={a for a in p.relocations if r<=a<r+z},n
 if kind=='code':code.update(range(r,r+z))
 rows.append(dict(unit=u,anchor=n,rva=r,size=z,section=s['section'],kind=kind,sha256=hashlib.sha256(raw).hexdigest(),object_sha256=hashlib.sha256(op.read_bytes()).hexdigest(),bindings=bindings))
# Independently preserve every accepted sibling in the actual existing owner.
old=[];fo=COFF(d/(font+'.obj'))
for r in c['regions']:
 if r['unit']!='d3dhook_ID3DXFontHook' or r['kind']!='code':continue
 assert 'DrawTextA@' not in r['anchor']
 s=next(s for s in fo.names[r['anchor']] if s['section']>0);sec=fo.sections[s['section']-1]
 assert sec['size']==r['size'] and sec['reloc_count']==0 and sec['bytes']==p.read(r['rva'],r['size'])
 old.append(dict(anchor=r['anchor'],rva=r['rva'],size=r['size']))
assert len(old)==17,len(old)
prior=accepted_code('evidence/checkpoint32/tcp-font-acceptance.json');assert len(prior)==351223 and len(code)==439 and len(code-prior)==439
# Unused proxy constructor and IUnknown/ID3DXSprite constructors have no incoming selected COFF fixup.
selected_names={b['symbol'] for r in rows for b in r['bindings']}
assert not any(n.startswith('??0') for n in selected_names)
out=dict(status='CANONICAL_OBJECT_PREFLIGHT_PASS_ANSI_EXACT_STRTOUL_LINK_PENDING',run=run,baseline=351223,new_unique_code=439,data_bytes=68,EH_bytes=0,direct025_code_credit=0,regions=rows,accepted_siblings_revalidated=old,new_imports={},proxy=dict(complete_size=4,vft_size=56,slot_count=14,whole_compiler_object_initialized_statically=True,CRT_sections=[],constructor9='Emitted but unused; excluded because static object already contains VFT relocation, no selected caller, multiple original homologues.'),unimplemented_runtime='StartupC4790 calloc(1,100001) for both char buffers remains unimplemented and uncredited. Both4-byte pointer owners are real original BSS, not substitute allocations.',owner_transfer='Canonical d3dhook_ID3DXFontHook v3 verified,17 accepted siblings intact; eval probe must not coexist in final link.',parser_dependency='ANSI271 now exact and all53 util siblings revalidated; new CRT strtoulC7DF5 requires archive/final-link qualification before acceptance.')
Path('build/agent-textdraw/font-hook-complete-reviewed-manifest.json').write_text(json.dumps(out,indent=2)+'\n')
Path('build/agent-textdraw/font-hook-complete-seeds.json').write_text(json.dumps([dict(unit=('d3dhook_ID3DXFontHook' if u==font else u),symbol=n,rva=r,size=z) for u,n,r,z,k in plan if k=='code'],indent=2)+'\n')
print('PASS439code+68data;',len(rows),'whole sections;',sum(len(r['bindings']) for r in rows),'bindings;',len(old),'accepted siblings unchanged; constructor9 excluded')
