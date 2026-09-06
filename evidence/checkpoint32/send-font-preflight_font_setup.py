from pathlib import Path
import sys,json,struct,hashlib
sys.path.insert(0,'tools')
from binary import PE,COFF
from rank_uncovered_candidates import accepted_code
D=Path('build/cp32-font-setup-trial1');OUT=Path('build/agent-textdraw');p=PE(Path('private/samp.dll'));c=json.loads(Path('config/checkpoint32/packet-registration-contract.json').read_text())
providers={r['anchor']:p.base+r['rva']+r.get('anchor_offset',0) for r in c['regions']};providers.update({n:v['reference_va'] for n,v in c['externals'].items()});providers['__except_list']=0
for mf in ['build/final-toggles-reviewed-manifest.json','build/agent-textdraw/transfer-layout-reviewed-manifest.json']:
 for row in json.loads(Path(mf).read_text())['regions']:providers[row['anchor']]=p.base+row['rva']+row.get('anchor_offset',0)
slots=[('scoreboard',0x26ec28),('chat_command',0x26ec24),('spawn',0x26ec2c),('application3',0x26ec30),('application4',0x26ec34),('transfer',0x26ec38)]
for name,r in slots:providers['?r5UiDialog_'+name+'@@3PAVCDXUTDialog@@A']=p.base+r
providers['?pUnkClass5@@3PAVCUnkClass5@@A']=p.base+0x26eb58
plan=[('closure_cmd_font_size','?cmdFontSize@@YAXPAD@Z',0x68670,164),('closure_refresh_ui_fonts','?RefreshApplicationUiFonts@@YAXXZ',0xc5430,106),('closure_setup_commands','?SetupCommands@@YAXXZ',0x69110,593)]
for u,n,r,z in plan:providers[n]=p.base+r
objects={u:COFF(D/(u+'.obj')) for u,n,r,z in plan};maps={u:{} for u in objects};rows={};seeds=[]
def add(u,si,r,anchor=None,kind=None,duplicate=False):
 o=objects[u];s=o.sections[si-1];assert si not in maps[u] or maps[u][si]==r;maps[u][si]=r
 if anchor is None:anchor=next(x['name'] for x in o.symbols.values() if x['section']==si and x['name']!=s['name'])
 a=next(x for x in o.names[anchor] if x['section']==si)
 rows[(u,si)]=dict(unit=u,anchor=anchor,anchor_offset=a['value'],rva=r,size=s['size'],section=si,kind=kind or ('code' if s['name'].startswith('.text') else 'data'),duplicate_previous=duplicate)
for u,n,r,z in plan:
 s=next(x for x in objects[u].names[n] if x['section']>0);assert objects[u].sections[s['section']-1]['size']==z;add(u,s['section'],r,n);seeds.append(dict(unit=u,symbol=n,rva=r,size=z))
stores=[('closure_store_ui_dialog_'+name,'?r5UiDialog_'+name+'@@3PAVCDXUTDialog@@A',r,4) for name,r in slots]+[('closure_store_transfer_window','?pUnkClass5@@3PAVCUnkClass5@@A',0x26eb58,4)]
for u,n,r,z in stores:
 objects[u]=COFF(D/(u+'.obj'));maps[u]={};a=next(x for x in objects[u].names[n] if x['section']>0);assert objects[u].sections[a['section']-1]['size']==z;add(u,a['section'],r,n)
# Discover only actual referenced, complete nonrelocating string/float sections. Providers are independently mapped above/baseline.
for (u,si),row in list(rows.items()):
 o=objects[u];sec=o.sections[si-1];r=row['rva']
 for i in range(sec['reloc_count']):
  off,idx,k=struct.unpack_from('<IIH',o.data,sec['reloc_ptr']+i*10);sym=o.symbols[idx];a=struct.unpack_from('<I',sec['bytes'],off)[0]
  if sym['section']>0 and sym['section'] not in maps[u] and (sym['name'].startswith('??_C@') or sym['name'].startswith('__real@')):
   v=struct.unpack('<I',p.read(r+off,4))[0];target=v if k==6 else (v+p.base+r+off+4)&0xffffffff;dr=target-p.base-a-sym['value'];ds=o.sections[sym['section']-1];assert ds['reloc_count']==0;assert ds['bytes']==p.read(dr,ds['size']);add(u,sym['section'],dr,sym['name'])
for (u,si),row in rows.items():
 o=objects[u];sec=o.sections[si-1];r=row['rva'];raw=bytearray(sec['bytes'] if not sec['uninitialized'] else bytes(sec['size']));bs=[]
 for i in range(sec['reloc_count']):
  off,idx,k=struct.unpack_from('<IIH',o.data,sec['reloc_ptr']+i*10);sym=o.symbols[idx];n=sym['name'];a=struct.unpack_from('<I',raw,off)[0]
  sv=p.base+maps[u][sym['section']]+sym['value'] if sym['section'] in maps[u] else providers[n]
  target=(sv+a)&0xffffffff;v=struct.unpack('<I',p.read(r+off,4))[0];actual=v if k==6 else (v+p.base+r+off+4)&0xffffffff;assert target==actual,(u,n,hex(r+off),hex(target),hex(actual));struct.pack_into('<I',raw,off,(target-(p.base+r+off+4 if k==20 else 0))&0xffffffff);bs.append(dict(symbol=n,site_rva=r+off,symbol_va=sv,addend=a,target_va=target,kind=k))
 
 try: expected=p.read(r,len(raw))
 except ValueError:
  containing=next(s for s in p.sections if s['rva']<=r and r+len(raw)<=s['rva']+s['virtual_size'])
  assert r>=containing['rva']+containing['size'];expected=bytes(len(raw))
 assert raw==expected,(u,hex(r),'bytes')
 assert {b['site_rva'] for b in bs if b['kind']==6 and b['symbol']!='__except_list'}=={x for x in p.relocations if r<=x<r+len(raw)},(u,hex(r),'PEreloc')
 row.update(bindings=bs,sha256=hashlib.sha256(raw).hexdigest(),object_sha256=hashlib.sha256((D/(u+'.obj')).read_bytes()).hexdigest(),whole_section=True)
for row in rows.values():
 if row['duplicate_previous']:
  old=[r for r in c['regions'] if r['rva']==row['rva'] and r['size']==row['size']]
  assert old and all(r['sha256']==row['sha256'] for r in old)
  row['previous_identical_owners']=[r['unit'] for r in old]
prior=accepted_code('evidence/checkpoint32/packet-registration-acceptance.json');union=set()
for r in rows.values():
 if r['kind']=='code':union.update(range(r['rva'],r['rva']+r['size']))
assert len(prior)==387041;assert len(union-prior)==863,len(union-prior)
out=dict(status='PASS_FULL_CODE_DATA_RELOCATIONS',run=D.name,baseline=len(prior),new_unique_code=863,source025_direct=0,regions=list(rows.values()),source_files={str(Path('client/saco')/(u+'.cpp')):hashlib.sha256((D/'client/saco'/(u+'.cpp')).read_bytes()).hexdigest() for u in objects},header_friend='Actual existing CChatWindow private CreateFonts symbol retained; only friend cmdFontSize added, no layout/access change.',outside_credit=['New pointers identify actual full existing UI objects; no startup allocation or lifetime credited.','Layout214 and finaltoggles36 are separately reviewed, outside863.'])
# The noop has many code-only homologues. Actual original Setup operand fixes its identity uniquely.
setup=next(r for r in rows.values() if r['anchor']=='?SetupCommands@@YAXXZ')
noop=next(b for b in setup['bindings'] if b['symbol']=='?cmdCmpStat@@YAXPAD@Z');assert noop['site_rva']==0x692fe and noop['target_va']==p.base+0x689f0
font=next(r for r in rows.values() if r['anchor']=='?cmdFontSize@@YAXPAD@Z');private=next(b for b in font['bindings'] if b['symbol'].startswith('?CreateFonts@CChatWindow'));assert private['symbol']=='?CreateFonts@CChatWindow@@AAEXXZ' and private['target_va']==p.base+0x681d0
out['noop_identity']=noop;out['private_provider_preserved']=private
(OUT/'font-setup-reviewed-manifest.json').write_text(json.dumps(out,indent=2)+'\n');(OUT/'font-setup-seeds.json').write_text(json.dumps(seeds,indent=2)+'\n');print(out['status'],len(rows),'sections',sum(len(r['bindings']) for r in rows.values()),'bindings',out['new_unique_code'],'unique')
