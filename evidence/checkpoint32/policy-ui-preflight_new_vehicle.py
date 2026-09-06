import sys,json,struct
from pathlib import Path
sys.path.insert(0,'tools');from binary import COFF,PE,sha,u32
from verify_checkpoint2 import relocs
p=PE('private/samp.dll');u='closure_game_new_vehicle';o=COFF('build/cp32-new-vehicle-trial1/'+u+'.obj');name='?NewVehicle@R5GameVehicleCreationView@@QAEPAVCVehicle@@HMMMMH@Z'
root=next(v for v in o.names[name] if v['section']>0);todo=[(root['section'],0xa0250)];seen={};rows=[];externals={'__except_list':0,'??2@YAPAXI@Z':p.base+0xc627a,'??3@YAXPAX@Z':p.base+0xc648a,'??0CVehicle@@QAE@HMMMMHH@Z':p.base+0xb83d0,'___CxxFrameHandler':p.base+0xc6526}
while todo:
 si,rva=todo.pop()
 if si in seen:assert seen[si]==rva;continue
 seen[si]=rva;sec=o.sections[si-1];original=p.read(rva,sec['size']);raw=bytearray(sec['bytes']);bindings=[]
 for f in relocs(o,si):
  off=f['offset'];s=f['symbol'];add=u32(raw,off);actual=u32(original,off)
  if f['kind']==20:actual=(actual+p.base+rva+off+4)&0xffffffff
  va=(actual-add)&0xffffffff
  if s['section']>0:todo.append((s['section'],va-p.base-s['value']))
  else:assert externals[s['name']]==va,(s['name'],hex(va))
  struct.pack_into('<I',raw,off,(actual-(p.base+rva+off+4 if f['kind']==20 else 0))&0xffffffff);bindings.append(dict(symbol=s['name'],site_rva=rva+off,kind=f['kind'],target_va=actual,symbol_va=va,addend=add))
 assert bytes(raw)==original
 assert {x for x in p.relocations if rva<=x<rva+sec['size']}=={b['site_rva'] for b in bindings if b['kind']==6 and b['symbol']!='__except_list'}
 syms=[s for s in o.symbols.values() if s['section']==si and s['name']!=sec['name']];anchor=next((s for s in syms if s['storage']==2),syms[0]);rows.append(dict(unit=u,anchor=anchor['name'],section=si,rva=rva,size=sec['size'],kind='code' if sec['flags']&0x20 else 'data',sha256=sha(original),object_sha256=sha(o.data),bindings=bindings))
prior=json.load(open('config/checkpoint32/rsa-commands-contract.json'));ctor=next(r for r in prior['regions'] if r['anchor']=='??0CVehicle@@QAE@HMMMMHH@Z');assert ctor['rva']==0xb83d0 and ctor['size']==1308
out=dict(status='WHOLE_VEHICLE_FACTORY142_EH21_DATA36_PREFLIGHT_PASS',regions=rows,expected_new_code=163,source025_direct=0,source025_role='Adapted real NewVehicle model-lifetime logic; R5 adds creationFlag and native-handle result filtering, so no direct025 credit.',allocation='Actual complete CVehicle184, verified static assertion and already accepted constructor1308. CGame pointer-only view never allocated.',state='Real this+6E+(model-400) keep-loaded byte;212 vehicle-model slots inherited from source and R5 offset. No extra validation/null guard or cleanup added.',providers=externals)
Path('build/new-vehicle-reviewed-manifest.json').write_text(json.dumps(out,indent=2)+'\n');Path('build/new-vehicle-seeds.json').write_text(json.dumps({u:{name:0xa0250}},indent=2)+'\n');print(out['status'],[(r['kind'],r['size']) for r in rows])
