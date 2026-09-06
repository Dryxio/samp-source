import sys,re,json,struct,hashlib,argparse
from pathlib import Path
sys.path.insert(0,'tools')
from binary import COFF,PE
ap=argparse.ArgumentParser();ap.add_argument('--run',required=True);args=ap.parse_args();run=Path('build')/args.run
p=PE(run/'closure.dll');o=COFF(run/'closure_chat_ime_interaction.obj');alias='_r5ImeShowReadingWindowImport';target='?s_bShowReadingWindow@CDXUTIMEEditBox@@1_NA';fun='?IsActive@R5ImeInteractionView@@SA_NXZ'
maps={}
for l in (run/'closure.map').read_text(errors='replace').splitlines():
 m=re.match(r'\s*[0-9a-fA-F]{4}:[0-9a-fA-F]{8}\s+(\S+)\s+([0-9a-fA-F]{8})\s',l)
 if m:maps[m.group(1)]=int(m.group(2),16)
assert '/alternatename:'+alias+'='+target in b''.join(s['bytes'] for s in o.sections if s['name']=='.drectve').decode('ascii')
owners=[]
for f in run.glob('*.obj'):
 ob=COFF(f)
 assert not any(s['section']>0 for s in ob.names.get(alias,[])),('fake alias storage',f)
 for s in ob.names.get(target,[]):
  if s['section']>0 and s['storage']==2:owners.append((f.name,s,ob.sections[s['section']-1]))
assert len(owners)==1 and owners[0][0]=='closure_ime_store_show_reading.obj'
assert owners[0][2]['size']==1 and owners[0][2]['uninitialized']
if alias in maps:assert maps[alias]==maps[target]
raws=next(s for s in o.names[fun] if s['section']>0);sec=o.sections[raws['section']-1];assert sec['size']==42;raw=bytearray(sec['bytes']);sites=set();alias_site=None
for i in range(sec['reloc_count']):
 off,si,k=struct.unpack_from('<IIH',o.data,sec['reloc_ptr']+10*i);n=o.symbols[si]['name'];actual=maps[target if n==alias else n];a=struct.unpack_from('<I',raw,off)[0];struct.pack_into('<I',raw,off,(actual+a-(maps[fun]+off+4 if k==20 else 0))&0xffffffff)
 if k==6:sites.add(maps[fun]-p.base+off)
 if n==alias:alias_site=maps[fun]+off;assert a==0
assert alias_site is not None and raw==p.read(maps[fun]-p.base,42)
assert sites=={x for x in p.relocations if maps[fun]-p.base<=x<maps[fun]-p.base+42}
assert struct.unpack('<I',p.read(alias_site-p.base,4))[0]==maps[target]
result=dict(status='ACTUAL_LINK_IMPORT_ALIAS_PASS',run=args.run,alias=alias,target=target,linked_target_va=maps[target],alias_operand_site=alias_site,sole_owner=owners[0][0],whole_IsActive42_bytes_and_PE_relocations=True,object_sha256=hashlib.sha256((run/'closure_chat_ime_interaction.obj').read_bytes()).hexdigest())
Path('build/agent-textdraw/chat-navigation-linked-alias-review.json').write_text(json.dumps(result,indent=2)+'\n');print(result)
