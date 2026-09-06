from pathlib import Path
import sys,json,struct,hashlib,argparse
sys.path.insert(0,'tools')
from binary import PE,COFF
from prepare_checkpoint32 import definition
from rank_uncovered_candidates import accepted_code
ap=argparse.ArgumentParser();ap.add_argument('--run',required=True);args=ap.parse_args();d=Path('build')/args.run
p=PE(Path('private/samp.dll'));c=json.loads(Path('config/checkpoint32/console-font-contract.json').read_text())
providers={r['anchor']:p.base+r['rva']-r.get('anchor_offset',0) for r in c['regions']+c['pending']};providers.update({n:v['reference_va'] for n,v in c['externals'].items()});providers['_sscanf']=p.base+0xca38b
script_rvas={'_get_car_z_angle':0xe9d98,'_get_actor_z_angle':0xe9dc0,'_get_active_interior':0xe9dac,'?r5CmdGetCarZAngle@@3USCRIPT_COMMAND@@B':0xe9d98,'?r5CmdGetActorZAngle@@3USCRIPT_COMMAND@@B':0xe9dc0};providers.update({n:p.base+r for n,r in script_rvas.items()})
plan=[('closure_cmd_save_positions','cmdSavePos',0x68a00,370,0),('closure_cmd_save_positions','cmdRawSavePos',0x68b80,382,0),('closure_cmd_weather_time','cmdSetWeather',0x68ed0,77,77),('closure_cmd_weather_time','cmdSetTime',0x68f20,162,162),('closure_cmd_show_interior','cmdShowInterior',0x68fd0,41,41)]
rows=[];data={};scripts={};union=set();source025=Path('/Users/salimtrouve/Documents/GitHub/samp-r5-preservation/references/michael-fa-samp/client/cmdprocs.cpp');reference025=source025.read_text();attribution=[]
for u,name,r,z,direct in plan:
 op=d/(u+'.obj');o=COFF(op);n='?'+name+'@@YAXPAD@Z';s=next(s for s in o.names[n] if s['section']>0);sec=o.sections[s['section']-1];assert sec['size']==z and s['value']==0;raw=bytearray(sec['bytes']);bindings=[]
 for i in range(sec['reloc_count']):
  off,si,k=struct.unpack_from('<IIH',o.data,sec['reloc_ptr']+10*i);sym=o.symbols[si];symbol=sym['name'];a=struct.unpack_from('<I',raw,off)[0];original=struct.unpack('<I',p.read(r+off,4))[0];orig_target=original if k==6 else (original+p.base+r+off+4)&0xffffffff
  if symbol.startswith('??_C@'):
   ds=o.sections[sym['section']-1];assert sym['value']==0 and ds['reloc_count']==0;dr=orig_target-p.base-a;assert ds['bytes']==p.read(dr,ds['size']),(symbol,'literal bytes');assert not any(dr<=x<dr+ds['size'] for x in p.relocations)
   data[(u,sym['section'])]=dict(unit=u,anchor=symbol,section=sym['section'],rva=dr,size=ds['size'],kind='data',sha256=hashlib.sha256(ds['bytes']).hexdigest(),bindings=[]);sv=p.base+dr
  else:sv=providers[symbol]
  target=(sv+a)&0xffffffff;assert target==orig_target,(name,symbol,hex(target),hex(orig_target));struct.pack_into('<I',raw,off,(target-(p.base+r+off+4 if k==20 else 0))&0xffffffff);bindings.append(dict(symbol=symbol,site_rva=r+off,symbol_va=sv,addend=a,target_va=target,kind=k))
  if symbol in script_rvas and sym['section']>0:
   ds=o.sections[sym['section']-1];sr=script_rvas[symbol];assert ds['bytes'][sym['value']:sym['value']+18]==p.read(sr,18);assert ds['reloc_count']==0;scripts[(u,symbol)]=dict(unit=u,anchor=symbol,section=sym['section'],offset=sym['value'],rva=sr,size=18,source_section_size=ds['size'],whole_section=ds['size']==18,sha256=hashlib.sha256(p.read(sr,18)).hexdigest())
 assert raw==p.read(r,z),name
 assert {b['site_rva'] for b in bindings if b['kind']==6}=={a for a in p.relocations if r<=a<r+z},(name,'PE')
 union.update(range(r,r+z));rows.append(dict(unit=u,anchor=n,rva=r,size=z,section=s['section'],kind='code',sha256=hashlib.sha256(raw).hexdigest(),object_sha256=hashlib.sha256(op.read_bytes()).hexdigest(),bindings=bindings))
 if direct:
  snapshot=d/'client/saco'/str(u+'.cpp');active=snapshot.read_text();a025=definition(reference025,name);actual=definition(active,name);assert actual==a025,(name,'025 direct definition differs');attribution.append(dict(name=name,code_bytes=direct,definition_sha256=hashlib.sha256(actual.encode()).hexdigest(),comparison='Exact full definition text after universal newline decoding, including signature; includes/prototypes outside body excluded.'))
# New split owners, once root has compiled them, are whole sections, never typed slices.
for u,n,sr in [('closure_store_cmd_car_z_angle','?r5CmdGetCarZAngle@@3USCRIPT_COMMAND@@B',0xe9d98),('closure_store_cmd_actor_z_angle','?r5CmdGetActorZAngle@@3USCRIPT_COMMAND@@B',0xe9dc0)]:
 op=d/(u+'.obj')
 if not op.exists():continue
 o=COFF(op);s=next(s for s in o.names[n] if s['section']>0);sec=o.sections[s['section']-1];assert sec['size']==18 and s['value']==0 and sec['reloc_count']==0 and sec['bytes']==p.read(sr,18)
 scripts[(u,n)]=dict(unit=u,anchor=n,section=s['section'],offset=0,rva=sr,size=18,source_section_size=18,whole_section=True,sha256=hashlib.sha256(sec['bytes']).hexdigest(),object_sha256=hashlib.sha256(op.read_bytes()).hexdigest())
complete=len(scripts)==3 and all(r['whole_section'] for r in scripts.values());prior=accepted_code('evidence/checkpoint32/console-font-acceptance.json');assert len(prior)==360128 and len(union-prior)==1032
out=dict(status='WHOLE_CODE_AND_LITERALS_PASS_'+('SCRIPT_STORES_COMPLETE' if complete else 'SPLIT_SCRIPT_STORES_PENDING'),run=args.run,baseline=len(prior),new_unique_code=1032,EH_bytes=0,source025_direct=280,source025_reference_sha256=hashlib.sha256(source025.read_bytes()).hexdigest(),direct_definitions=attribution,hybrid_save_raw_credit=0,regions=rows+list(data.values()),script_objects=list(scripts.values()),data_bytes=sum(r['size'] for r in data.values())+54,new_CRT={'_sscanf':dict(reference_va=p.base+0xca38b,actual_archive_and_final_link_qualification_pending=True)},excluded=['FindPlayerPed94 is an already-covered helper, not new code.','Unused UNASSIGNED shared16 data and template constants excluded.','AddEntry/PushBack remain implemented but unmatched descendants of accepted chat providers; no credit.'],real_layout='Uses complete existing CPlayerPed, CEntity, PED_TYPE and VEHICLE_TYPE; no allocation/view-size substitution. m_pPed2A4,m_dwGTAId44, matrix14, native model22, colors434/435.',runtime='No commands or game operations executed; source compilation and static matching only.')
for row in out['regions']+out['script_objects']:
 row['object_sha256']=hashlib.sha256((d/(row['unit']+'.obj')).read_bytes()).hexdigest()
 if row in out['script_objects']:
  row['kind']='data';row['bindings']=[]
  assert not any(row['rva']<=x<row['rva']+row['size'] for x in p.relocations)
out['source_files']={str(Path('client/saco')/(u+'.cpp')):hashlib.sha256((d/'client/saco'/(u+'.cpp')).read_bytes()).hexdigest() for u in {r['unit'] for r in out['regions']+out['script_objects']}}
Path('build/agent-textdraw/cmd-save-weather-reviewed-manifest.json').write_text(json.dumps(out,indent=2)+'\n');Path('build/agent-textdraw/cmd-save-weather-seeds.json').write_text(json.dumps([dict(unit=u,symbol='?'+n+'@@YAXPAD@Z',rva=r,size=z) for u,n,r,z,_ in plan],indent=2)+'\n');print(out['status'],len(rows),'code',sum(len(r['bindings']) for r in rows),'bindings',len(data),'whole strings;025 exactdefinitions280')
