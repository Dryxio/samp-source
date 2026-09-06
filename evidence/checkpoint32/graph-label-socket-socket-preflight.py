import json,sys,struct
from pathlib import Path
sys.path.insert(0,'tools')
from binary import COFF,PE,sha,u32
from verify_checkpoint2 import import_slots
p=PE('private/samp.dll');o=COFF('build/cp32-socket-leaves-trial1/closure_vendor_socket_leaves.obj');inv={f['rva']:f for f in json.load(open('config/checkpoint31/inventory.json'))['functions']};slots=import_slots(p)
seeds={};regions=[];external={}
for row in json.load(open('build/cp32-socket-leaves-trial1/discovery.json'))['rows']:
 if not row['candidates']:continue
 assert len(row['candidates'])==1
 c=row['candidates'][0];rva=c['rva'];symbol=row['symbol'];sym=next(s for s in o.names[symbol] if s['section']>0);sec=o.sections[sym['section']-1]
 assert len(inv[rva]['chunks'])==1 and inv[rva]['chunks'][0]['size']==sec['size']==row['size'] and sym['value']==0
 raw,fixes=o.function(symbol);patched=bytearray(raw);original=p.read(rva,len(raw));bindings=[]
 for f in fixes:
  at=f['offset'];name=f['symbol']['name'];assert f['kind']==6 and name.startswith('__imp_')
  va=u32(original,at)-u32(raw,at);assert va==c['inferred_targets'][name] and va in slots
  identity=slots[va];assert identity[0].lower()=='wsock32.dll'
  external[name]={'reference_va':va,'import':list(identity)};struct.pack_into('<I',patched,at,va+u32(raw,at));bindings.append(dict(symbol=name,site_rva=rva+at,kind=6,target_va=va+u32(raw,at)))
 assert patched==original
 assert {v-rva for v in p.relocations if rva<=v<rva+len(raw)}=={f['offset'] for f in fixes}
 seeds[symbol]=rva;regions.append(dict(unit='closure_vendor_socket_leaves',anchor=symbol,section=sym['section'],rva=rva,size=len(raw),kind='.text',sha256=sha(original),bindings=bindings,whole_bytes_equal=True))
assert sum(r['size'] for r in regions)==427
out=dict(status='WHOLE_SECTION_PREFLIGHT_ALL_IMPORT_BINDINGS_VERIFIED',run='cp32-socket-leaves-trial1',seeds={'closure_vendor_socket_leaves':seeds},regions=regions,imports=external,code_bytes=427,direct025_credit=0,EH=0,data_bytes=0,source='Unchanged normal vendor SocketLayer function bodies with true native sockaddr_in sizes; no SocketLayer allocation/state or socket execution.',excluded='SendTo vendor118 differs from R5 body with additional packet transformation and global buffer; no shorter-prefix matching or credit. Other unselected methods absent.')
Path('build/socket-leaves-reviewed-manifest.json').write_text(json.dumps(out,indent=2)+'\n');print(len(regions),sum(len(r['bindings']) for r in regions),'bindings',external)
