import json,sys,struct,hashlib
from pathlib import Path
sys.path.insert(0,'tools');from binary import PE,COFF,sha
from verify_checkpoint2 import map_symbols
import argparse
ap=argparse.ArgumentParser();ap.add_argument('--run',default='cp32-console-font-linked1');run=ap.parse_args().run;lib=Path('build/libcmt.lib').read_bytes();pin=json.load(open('config/checkpoint31/sdk.json'))['libcmt.lib'];assert sha(lib)==pin
at=8;names=b'';members=[]
while at<len(lib):
 h=lib[at:at+60];n=h[:16].decode().strip();size=int(h[48:58]);raw=lib[at+60:at+60+size];at+=60+size+(size%2)
 if n=='//':names=raw;continue
 if n.startswith('/') and n[1:].isdigit():
  off=int(n[1:]);n=names[off:names.index(b'\0',off)].decode()
 if n.lower().endswith('nlsdata1.obj'):members.append((n,raw))
assert len(members)==1;Path('build/nlsdata1.obj').write_bytes(members[0][1]);o=COFF('build/nlsdata1.obj');s=o.names['___mb_cur_max'][0];sec=o.sections[s['section']-1];assert s['value']==0 and sec['name']=='.data' and sec['size']==12 and sec['reloc_count']==0 and sec['bytes']==struct.pack('<III',1,0x2e,1)
expected={'___mb_cur_max':0,'___decimal_point':4,'___decimal_point_length':8};assert {v['name']:v['value'] for v in o.symbols.values() if v['section']==s['section'] and v['storage']==2}==expected
p=PE('private/samp.dll');q=PE('build/'+run+'/closure.dll');maps=map_symbols(Path('build')/run/'closure.map');actual=maps['___mb_cur_max'][0][0]
for n,off in expected.items():assert maps[n]==[(actual+off,'LIBCMT:nlsdata1.obj')]
for pe,rva in [(p,0x117668),(q,actual-q.base)]:assert pe.read(rva,12)==sec['bytes'] and not any(rva<=x<rva+12 for x in pe.relocations)
x=dict(status='PINNED_COMPLETE_CRT_LOCALE_DATA12_QUALIFIED',run=run,library_sha256=pin,member_path=members[0][0],member_sha256=sha(o.data),section=dict(index=s['section'],name=sec['name'],size=12,sha256=sha(sec['bytes']),relocations=0,symbol_offsets=expected),original_rva=0x117668,linked_va=actual,linked_sha256=sha(q.data),data_and_code_credit=0,scope='Complete real initialized archive block: int mb_cur_max, decimal_point with compiler alignment, int decimal_point_length. No duplicate source storage.')
Path('build/crt-locale-data-qualified.json').write_text(json.dumps(x,indent=2)+'\n');print('PASS actual SDK archive block12, allthree symbols and both PE images')
