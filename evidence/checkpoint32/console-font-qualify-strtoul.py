import json,sys,struct
from pathlib import Path
sys.path.insert(0,'tools');from binary import PE,sha
from verify_checkpoint2 import map_symbols
run='cp32-console-font-linked1';d=Path('build')/run;p=PE('private/samp.dll');q=PE(d/'closure.dll');maps=map_symbols(d/'closure.map');sdk=json.load(open(d/'link.json',encoding='utf-8-sig'))['sdk'];assert sdk['libcmt.lib']==json.load(open('config/checkpoint31/sdk.json'))['libcmt.lib']
def addr(n):
 hits=maps[n];assert len(hits)==1 and hits[0][1].lower()=='libcmt:strtol.obj';return hits[0][0]
a=addr('_strtoul');b=addr('_strtoxl');raw=bytearray(p.read(0xc7df5,23));assert raw[14]==0xe8 and 0xc7df5+19+struct.unpack_from('<i',raw,15)[0]==0xc7c1f
assert not any(0xc7df5<=r<0xc7df5+23 for r in p.relocations) and not any(a-q.base<=r<a-q.base+23 for r in q.relocations)
struct.pack_into('<i',raw,15,b-(a+19));assert bytes(raw)==q.read(a-q.base,23)
inv=json.load(open('config/checkpoint31/inventory.json'));provider=next(f for f in inv['functions'] if f['rva']==0xc7c1f);assert provider['name']=='_strtoxl'
x=dict(status='PINNED_CRT_STRTOUL_WHOLE23_REAL_STRTOXL_BOUNDARY_QUALIFIED',run=run,linked_sha256=sha(q.data),sdk_libcmt_sha256=sdk['libcmt.lib'],new_source_credit=0,code=dict(symbol='_strtoul',original_rva=0xc7df5,size=23,linked_va=a,whole_resolved_bytes_equal=True,all_PE_relocations_equal=True,fixup=dict(offset=15,kind='REL32',original_target_rva=0xc7c1f,linked_target_va=b)),provider=dict(symbol='_strtoxl',original_rva=0xc7c1f,linked_va=b,owner='LIBCMT:strtol.obj',scope='True pinned CRT internal boundary, not reconstructed source coverage'))
Path('build/console-font-strtoul-qualified.json').write_text(json.dumps(x,indent=2)+'\n');print('PASS strtoul23, true strtoxl SDK target, zero credit')
