import json,sys,struct
from pathlib import Path
sys.path.insert(0,'tools');from binary import COFF,PE,sha
from verify_checkpoint2 import relocs
p=PE('private/samp.dll');rows=[];seeds={}
providers={'?r5FontCharacterIndex@@3HA':0x136ea4,'?r5FontOriginalText@@3PADA':0x140aec,'?r5FontEmbeddedColor@@3KA':0x136e90,'?GetColorFromEmbedCode@@YAKPAD@Z':0xb5e80,'___mb_cur_max':0x117668,'_mblen':0xca3bf,'??_7R5FontSpriteProxy@@6B@':0xea744}
plans=[('cp32-font-ansi-consumer-trial1','closure_font_ansi_consumer','?R5FontConsumeAnsiColorEmbeds@@YAXXZ',0x73a10,201),('cp32-console-font-ob1-v3','closure_font_sprite_proxy','??0R5FontSpriteProxy@@QAE@XZ',0x73ec0,9)]
for run,u,n,r,z in plans:
 o=COFF(Path('build')/run/(u+'.obj'));s=next(v for v in o.names[n] if v['section']>0);sec=o.sections[s['section']-1];raw=bytearray(sec['bytes']);assert sec['size']==z and s['value']==0;bindings=[]
 for f in relocs(o,s['section']):
  off=f['offset'];symbol=f['symbol']['name'];add=struct.unpack_from('<I',raw,off)[0];va=p.base+providers[symbol];target=(va+add)&0xffffffff;value=target-(p.base+r+off+4 if f['kind']==20 else 0);struct.pack_into('<I',raw,off,value&0xffffffff);bindings.append(dict(symbol=symbol,site_rva=r+off,symbol_va=va,addend=add,target_va=target,kind=f['kind']))
 assert bytes(raw)==p.read(r,z);assert {r+f['offset'] for f in relocs(o,s['section']) if f['kind']==6}=={x for x in p.relocations if r<=x<r+z}
 rows.append(dict(unit=u,anchor=n,section=s['section'],rva=r,size=z,kind='code',sha256=sha(bytes(raw)),object_sha256=sha(o.data),bindings=bindings));seeds.setdefault(u,{})[n]=r
 if z==9:
  ds=json.load(open('build/cp32-font-hook-trial1/discovery-attached.json'));ds=ds['rows'] if isinstance(ds,dict) else ds
  row=next(x for x in ds if x['unit']==u and x['symbol']==n)
  candidates=row['candidates'];qualified=[]
  for c in candidates:
   at=c['rva'] if isinstance(c,dict) else int(c,0) if isinstance(c,str) else c
   if p.read(at,9)==bytes(raw):qualified.append(at)
  assert qualified==[r],qualified
out=dict(status='WHOLE_ANSI201_AND_UNIQUE_VFT_CONSTRUCTOR9_PREFLIGHT_PASS',regions=rows,new_code_expected=210,new_direct025=0,constructor_identity='All raw candidate bodies tested with fixed previously accepted VFT EA744, only73EC0 survives. C4780 targets another VFT ED8F0. No caller/lifetime allocation claim.',CRT_data_scope='Entire real nlsdata1.obj12 block qualified separately; mblen137 pinned CRT needs link qualification; both receive zero credit.',runtime_limit='Original font heap buffers startup allocation remains unimplemented and excluded.')
Path('build/font-ansi-consumer-reviewed.json').write_text(json.dumps(out,indent=2)+'\n');Path('build/font-ansi-consumer-seeds.json').write_text(json.dumps(seeds,indent=2)+'\n');print('PASS201+9 full bytes and PE, constructor unique fixed VFT')
