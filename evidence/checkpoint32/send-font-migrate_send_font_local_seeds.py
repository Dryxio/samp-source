import json,sys,re,struct
from pathlib import Path
sys.path.insert(0,'tools');from binary import COFF,sha
def relocs(o,si):
 s=o.sections[si-1];out=[]
 for i in range(s['reloc_count']):
  off,idx,kind=struct.unpack_from('<IIH',o.data,s['reloc_ptr']+10*i);out.append(dict(offset=off,kind=kind,symbol=o.symbols[idx]))
 return out
u='closure_menu_pool_process';old=COFF('build/cp32-packet-registration-linked1/'+u+'.obj');new=COFF('build/cp32-send-font-linked1/'+u+'.obj');pairs={'$L95068':'$L95070','$T95086':'$T95088'};proof=[]
# Whole object section topology, bytes and relocation identity must be unchanged.
assert len(old.sections)==len(new.sections)
for si,(a,b) in enumerate(zip(old.sections,new.sections),1):
 if a['name']=='.debug$S':
  assert b['name']=='.debug$S' and a['size']==b['size'];continue # Non-linked CodeView records include different compiler cache-object paths.
 assert (a['name'],a['size'],a['flags'],a['bytes'])==(b['name'],b['size'],b['flags'],b['bytes'])
 ar=relocs(old,si);br=relocs(new,si);assert len(ar)==len(br)
 for x,y in zip(ar,br):
  assert (x['offset'],x['kind'])==(y['offset'],y['kind']);sx,sy=x['symbol'],y['symbol']
  assert (sx['section'],sx['value'],sx['storage'])==(sy['section'],sy['value'],sy['storage'])
  if sx['name']!=sy['name']:assert re.fullmatch(r'\$[LT]\d+',sx['name']) and re.fullmatch(r'\$[LT]\d+',sy['name'])
for a,b in pairs.items():
 x=old.names[a][0];y=new.names[b][0];assert (x['section'],x['value'],x['storage'])==(y['section'],y['value'],y['storage']);assert a not in new.names
 proof.append(dict(old=a,new=b,section=x['section'],offset=x['value'],whole_size=old.sections[x['section']-1]['size']))
p=Path('config/checkpoint32/send-font-seeds.json');seeds=json.loads(p.read_text())
for a,b in pairs.items():assert b not in seeds[u];seeds[u][b]=seeds[u].pop(a)
p.write_text(json.dumps(seeds,indent=2)+'\n')
Path('build/send-font-local-seed-migration.json').write_text(json.dumps(dict(status='PASS_ALL_LINKABLE_SECTIONS_AND_RELOCATION_IDENTITIES',unit=u,old_run='cp32-packet-registration-linked1',new_run='cp32-send-font-linked1',old_object_sha256=sha(old.data),new_object_sha256=sha(new.data),migrations=proof,code_credit=0),indent=2)+'\n');print('PASS two compiler-local seed renames; all linkable sections and relocation graph unchanged')
