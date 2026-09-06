import json,sys,re,struct
from pathlib import Path
sys.path.insert(0,'tools');from binary import PE,sha
run='cp32-connection-graph-linked1';p=PE('private/samp.dll');q=PE('build/'+run+'/closure.dll');m=Path('build/'+run+'/closure.map').read_text();inv=json.load(open('config/checkpoint31/inventory.json'))
rows=[];mapping={}
for n,r in [('__itoa',0xc8b55),('_xtoa',0xc8b17)]:
 match=re.search(r'\s'+re.escape(n)+r'\s+([0-9A-Fa-f]{8})\s+f\s+LIBCMT:xtoa.obj',m);assert match,n
 address=int(match.group(1),16);mapping[r]=address-q.base
 f=next(f for f in inv['functions'] if f['rva']==r);assert len(f['chunks'])==1
 rows.append(dict(symbol=n,original_rva=r,linked_rva=address-q.base,size=f['chunks'][0]['size'],SDK_member='LIBCMT:xtoa.obj'))
for row in rows:
 r=row['original_rva'];s=row['linked_rva'];raw=bytearray(p.read(r,row['size']));fix=[]
 if row['symbol']=='__itoa':
  assert raw[30]==0xe8;assert (r+35+struct.unpack_from('<i',raw,31)[0])==0xc8b17
  struct.pack_into('<i',raw,31,mapping[0xc8b17]-(s+35));fix=[dict(offset=31,original_target_rva=0xc8b17,linked_target_rva=mapping[0xc8b17])]
 assert not {x for x in p.relocations if r<=x<r+row['size']}
 assert not {x for x in q.relocations if s<=x<s+row['size']}
 assert raw==q.read(s,row['size']),row
 row.update(whole_resolved_equal=True,bindings=fix)
Path('build/connection-graph-itoa-qualified.json').write_text(json.dumps(dict(status='PINNED_CRT_PROVIDER_FULL_BYTES_QUALIFIED',run=run,linked_sha256=sha(q.data),new_source_credit=0,code=rows),indent=2)+'\n');print('PASS __itoa42 + true _xtoa62, no source credit')
