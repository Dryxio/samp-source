import sys,json,struct,hashlib
from pathlib import Path
sys.path.insert(0,'tools')
from binary import PE,COFF
p=PE(Path('private/samp.dll'));d=Path('build/cp32-scoreboard-draw-trial1');o=COFF(d/'closure_scoreboard_request.obj');c=json.load(open('config/checkpoint32/lifecycle-chat-contract.json'));ps={r['anchor']:p.base+r['rva']-r.get('anchor_offset',0) for r in c['regions']};ps.update({n:v['reference_va'] for n,v in c['externals'].items()});ps.update({'__except_list':0,'?r5ScoreboardLastRequestTick@@3KA':0x10118a18,'?r5RpcRequestScoresPings@@3HB':0x100e5b28})
placements={46:0x8f10,47:0xe0270,48:0xf7a5c};rows=[]
for s in o.symbols.values():
 if s['section'] in placements:ps[s['name']]=p.base+placements[s['section']]+s['value']
for idx,r in placements.items():
 sec=o.sections[idx-1];raw=bytearray(sec['bytes']);bs=[]
 for i in range(sec['reloc_count']):
  off,si,k=struct.unpack_from('<IIH',o.data,sec['reloc_ptr']+10*i);n=o.symbols[si]['name'];add=struct.unpack_from('<I',raw,off)[0];sv=ps[n];va=(sv+add)&0xffffffff;site=r+off;orig=struct.unpack('<I',p.read(site,4))[0];assert (orig if k==6 else (orig+p.base+site+4)&0xffffffff)==va,n
  struct.pack_into('<I',raw,off,(va-(p.base+site+4 if k==20 else 0))&0xffffffff);bs.append(dict(symbol=n,site_rva=site,symbol_va=sv,addend=add,target_va=va,kind=k))
 assert raw==p.read(r,len(raw));assert {b['site_rva'] for b in bs if b['kind']==6 and b['symbol']!='__except_list'}=={x for x in p.relocations if r<=x<r+len(raw)}
 anchor=next(s['name'] for s in o.symbols.values() if s['section']==idx and s['value']==0 and not s['name'].startswith('.'))
 rows.append(dict(unit='closure_scoreboard_request',anchor=anchor,section=idx,rva=r,size=len(raw),kind=sec['name'],sha256=hashlib.sha256(raw).hexdigest(),whole_bytes_equal=True,bindings=bs))
for u,n,r in [('closure_store_scoreboard_request_tick','?r5ScoreboardLastRequestTick@@3KA',0x118a18),('closure_store_scoreboard_request_rpc','?r5RpcRequestScoresPings@@3HB',0xe5b28)]:
 a=COFF(d/(u+'.obj'));s=next(s for s in a.names[n] if s['section']>0);sec=a.sections[s['section']-1];raw=bytes(sec['size']) if sec['uninitialized'] else sec['bytes'];assert len(raw)==4 and raw==p.read(r,4);assert not sec['reloc_count'];rows.append(dict(unit=u,anchor=n,section=s['section'],rva=r,size=4,kind=sec['name'],sha256=hashlib.sha256(raw).hexdigest(),whole_bytes_equal=True,bindings=[]))
Path('build/agent-textdraw/scoreboard-request-reviewed-manifest.json').write_text(json.dumps(dict(status='REVIEWED_SCOPED_FUNCTION_PREFLIGHT_NOT_ACCEPTANCE',regions=rows,code_bytes=172,data_bytes=44,direct025_credit=0,excluded_header_sentinel_bytes=16),indent=2)+'\n');print('PASS172code+44data',sum(len(r['bindings']) for r in rows),'bindings')
