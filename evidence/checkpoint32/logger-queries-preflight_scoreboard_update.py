import sys,json,struct,hashlib
from pathlib import Path
sys.path.insert(0,'tools')
from binary import PE,COFF
p=PE(Path('private/samp.dll')); path=Path('build/cp32-scoreboard-update-trial3/closure_scoreboard_update.obj');o=COFF(path)
c=json.load(open('config/checkpoint32/lifecycle-chat-contract.json'))
ps={r['anchor']:p.base+r['rva']-r.get('anchor_offset',0) for r in c['regions']};ps.update({n:v['reference_va'] for n,v in c['externals'].items()})
q=json.load(open('build/agent-textdraw/scoreboard-player-queries-reviewed-manifest.json'))
for fn in ['scoreboard-player-queries-reviewed-manifest.json','listbox-items-reviewed-manifest.json']:
 for r in json.load(open('build/agent-textdraw/'+fn))['regions']:ps[r['anchor']]=p.base+r['rva']
strings={'??_C@_02GMHACPFF@?$CFu?$AA@':(0xe9344,b'%u\0'),'??_C@_02DPKJAMEF@?$CFd?$AA@':(0xea6ac,b'%d\0')}
rows=[]
for n,(r,expected) in strings.items():
 s=next(x for x in o.names[n] if x['section']>0);sec=o.sections[s['section']-1]
 assert sec['bytes']==expected==p.read(r,len(expected));assert not sec['reloc_count'];ps[n]=p.base+r
 rows.append(dict(unit='closure_scoreboard_update',anchor=n,section=s['section'],rva=r,size=len(expected),sha256=hashlib.sha256(expected).hexdigest(),kind='.rdata',whole_bytes_equal=True,bindings=[]))
n='?UpdatePlayers@R5ScoreboardUpdateView@@QAEXXZ';r=0x6ed30;s=next(x for x in o.names[n] if x['section']>0);sec=o.sections[s['section']-1];raw=bytearray(sec['bytes']);assert len(raw)==892;bindings=[]
for i in range(sec['reloc_count']):
 off,si,k=struct.unpack_from('<IIH',o.data,sec['reloc_ptr']+10*i);name=o.symbols[si]['name'];add=struct.unpack_from('<I',raw,off)[0]
 if name not in ps:raise Exception('missing '+name)
 sv=ps[name];va=(sv+add)&0xffffffff;site=r+off;old=struct.unpack('<I',p.read(site,4))[0]
 assert (old if k==6 else (old+p.base+site+4)&0xffffffff)==va,(name,hex(va),hex(old))
 struct.pack_into('<I',raw,off,(va-(p.base+site+4 if k==20 else 0))&0xffffffff)
 bindings.append(dict(symbol=name,site_rva=site,symbol_va=sv,addend=add,target_va=va,kind=k))
assert raw==p.read(r,892)
assert {b['site_rva'] for b in bindings if b['kind']==6}=={x for x in p.relocations if r<=x<r+892}
assert not any(x['name'] in ('.text$x','.xdata$x') and x['size'] for x in o.sections)
rows.insert(0,dict(unit='closure_scoreboard_update',anchor=n,section=s['section'],rva=r,size=892,sha256=hashlib.sha256(raw).hexdigest(),kind='.text',whole_bytes_equal=True,bindings=bindings))
rows+=q['regions']
Path('build/agent-textdraw/scoreboard-update-reviewed-manifest.json').write_text(json.dumps(dict(status='REVIEWED_SCOPED_FUNCTION_PREFLIGHT_NOT_ACCEPTANCE',regions=rows,code_bytes=1324,data_bytes=6,EH=0,direct025_credit=0,buffer_capacity_inference={'physical_extent':260,'nonunique_declared_capacity_interval':[257,260]},object_sha256=hashlib.sha256(path.read_bytes()).hexdigest()),indent=2)+'\n')
print('PASS',len(bindings),'fixups',len(rows),'regions; 1324 code; whole strings6; EH0')
