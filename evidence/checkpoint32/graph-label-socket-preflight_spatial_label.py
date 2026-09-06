import sys,json,struct,hashlib
from pathlib import Path
sys.path.insert(0,'tools')
from binary import PE,COFF
p=PE(Path('private/samp.dll'));d=Path('build/cp32-spatial-label-trial1');c=json.load(open('config/checkpoint32/lifecycle-chat-contract.json'));ps={r['anchor']:p.base+r['rva']-r.get('anchor_offset',0) for r in c['regions']};ps.update({n:v['reference_va'] for n,v in c['externals'].items()})
ps['?pcaInternalAim@@3PAU_CAMERA_AIM@@A']=0x101039c8
spec=[('closure_spatial_label_renderer','??0R5SpatialLabelRenderer@@QAE@PAUIDirect3DDevice9@@@Z',0x6bbb0,25),('closure_spatial_label_renderer','??1R5SpatialLabelRenderer@@QAE@XZ',0x6bbd0,28),('closure_spatial_label_renderer','?OnLostDevice@R5SpatialLabelRenderer@@QAEXXZ',0x6bbf0,14),('closure_spatial_label_renderer','?OnResetDevice@R5SpatialLabelRenderer@@QAEXXZ',0x6bc00,14),('closure_spatial_label_renderer','?IsLineOfSightClear@R5SpatialLabelRenderer@@QAEHMMM@Z',0x6bc10,96),('closure_spatial_label_renderer','?Begin@R5SpatialLabelRenderer@@QAEXXZ',0x6bc70,16),('closure_spatial_label_renderer','?End@R5SpatialLabelRenderer@@QAEXXZ',0x6bc80,14),('closure_spatial_label_renderer','?Draw@R5SpatialLabelRenderer@@QAEXPAUD3DXVECTOR3@@PADKH_N@Z',0x6bc90,351),('closure_label_internal_aim','?GameGetInternalAim@@YGPAU_CAMERA_AIM@@XZ',0x9c980,6),('closure_store_label_line_of_sight','?r5LabelLineOfSight@@3USCRIPT_COMMAND@@B',0xea618,18),('closure_spatial_label_renderer','__real@3f800000',0xe5b2c,4)]
ps.update({n:p.base+r for u,n,r,z in spec});objects={u:COFF(d/(u+'.obj')) for u,n,r,z in spec};rows=[]
for u,n,r,z in spec:
 o=objects[u];s=next(s for s in o.names[n] if s['section']>0);sec=o.sections[s['section']-1];raw=bytearray(sec['bytes']);assert len(raw)==z and s['value']==0;bs=[]
 for i in range(sec['reloc_count']):
  off,si,k=struct.unpack_from('<IIH',o.data,sec['reloc_ptr']+10*i);name=o.symbols[si]['name'];add=struct.unpack_from('<I',raw,off)[0];sv=ps[name];va=(sv+add)&0xffffffff;site=r+off;orig=struct.unpack('<I',p.read(site,4))[0];assert (orig if k==6 else (orig+p.base+site+4)&0xffffffff)==va,(name,hex(va))
  struct.pack_into('<I',raw,off,(va-(p.base+site+4 if k==20 else 0))&0xffffffff);bs.append(dict(symbol=name,site_rva=site,symbol_va=sv,addend=add,target_va=va,kind=k))
 assert raw==p.read(r,z);assert {b['site_rva'] for b in bs if b['kind']==6}=={x for x in p.relocations if r<=x<r+z};rows.append(dict(unit=u,anchor=n,section=s['section'],rva=r,size=z,kind=sec['name'],sha256=hashlib.sha256(raw).hexdigest(),whole_bytes_equal=True,bindings=bs))
for u,o in objects.items():assert not any(s['name'] in ('.text$x','.xdata$x') and s['size'] for s in o.sections)
assert p.read(0x1039c8,4)==bytes.fromhex('2cf3b600')
Path('build/agent-textdraw/spatial-label-reviewed-manifest.json').write_text(json.dumps(dict(status='REVIEWED_SCOPED_FUNCTION_PREFLIGHT_NOT_ACCEPTANCE',regions=rows,code_bytes=564,new_data_maximum=18,existing_float_reused=4,existing_pcaInternalAim_owner='closure_store_internal_aim',EH=0,direct025_code_credit=0,missing025_descriptor_data=18,excluded_helpers=['D3DXVECTOR3 constructor3','D3DXMATRIX constructor3','D3DXMatrixIdentity59'],objects={u:hashlib.sha256((d/(u+'.obj')).read_bytes()).hexdigest() for u in objects}),indent=2)+'\n');print('PASS564code+18newdescriptor; 11regions',sum(len(r['bindings']) for r in rows),'bindings')
