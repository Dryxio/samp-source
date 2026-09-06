from pathlib import Path
import sys,json,struct,hashlib
sys.path.insert(0,'tools')
from binary import PE,COFF
p=PE(Path('private/samp.dll'));run='cp32-tcp-font-trial1';u='closure_font_color_helpers';op=Path('build')/run/(u+'.obj');o=COFF(op)
providers={'__imp__MultiByteToWideChar@24':p.base+0xe5200,'?r5FontCharacterIndex@@3HA':p.base+0x136ea4,'?r5FontWideText@@3PAGA':p.base+0x136ea8,'?r5FontEmbeddedColor@@3KA':p.base+0x136e90,'?r5FontOriginalText@@3PADA':p.base+0x140aec,'?GetColorFromEmbedCode@@YAKPAG@Z':p.base+0xb5f90}
plan=[('?R5FontConvertAnsiToWide@@YAXPADPAGH@Z',0x739a0,108),('?R5FontConsumeWideColorEmbeds@@YAXXZ',0x73ae0,223)]
# Confirm the actual original import slot rather than trusting its name/address.
opt=struct.unpack_from('<I',p.data,0x3c)[0]+24;ird=struct.unpack_from('<I',p.data,opt+104)[0]
def text(r):
 out=bytearray()
 while p.read(r+len(out),1)!=b'\0':out+=p.read(r+len(out),1)
 return out.decode('ascii')
found=[]
while True:
 ilt,_,_,dll,iat=struct.unpack('<IIIII',p.read(ird,20))
 if not any((ilt,dll,iat)):break
 index=0
 while True:
  entry=struct.unpack('<I',p.read((ilt or iat)+index*4,4))[0]
  if not entry:break
  if not entry&0x80000000 and text(entry+2)=='MultiByteToWideChar':found.append((text(dll),iat+index*4))
  index+=1
 ird+=20
assert found==[('KERNEL32.dll',0xe5200)],found
contract=json.loads(Path('config/checkpoint32/replica-navigation-contract.json').read_text())
wide=next(r for r in contract['regions'] if r['anchor']=='?GetColorFromEmbedCode@@YAKPAG@Z');assert wide['rva']==0xb5f90
rows=[]
for n,r,z in plan:
 s=next(s for s in o.names[n] if s['section']>0);sec=o.sections[s['section']-1];assert sec['size']==z and s['value']==0;raw=bytearray(sec['bytes']);bindings=[]
 for i in range(sec['reloc_count']):
  off,si,k=struct.unpack_from('<IIH',o.data,sec['reloc_ptr']+10*i);sym=o.symbols[si]['name'];a=struct.unpack_from('<I',raw,off)[0];sv=providers[sym];target=(sv+a)&0xffffffff;original=struct.unpack('<I',p.read(r+off,4))[0];assert target==(original if k==6 else (original+p.base+r+off+4)&0xffffffff)
  struct.pack_into('<I',raw,off,(target-(p.base+r+off+4 if k==20 else 0))&0xffffffff);bindings.append(dict(symbol=sym,site_rva=r+off,symbol_va=sv,addend=a,target_va=target,kind=k))
 assert raw==p.read(r,z)
 assert {b['site_rva'] for b in bindings if b['kind']==6}=={x for x in p.relocations if r<=x<r+z}
 rows.append(dict(unit=u,anchor=n,section=s['section'],rva=r,size=z,kind='code',sha256=hashlib.sha256(raw).hexdigest(),object_sha256=hashlib.sha256(op.read_bytes()).hexdigest(),bindings=bindings))
assert not any(r['rva']<=0x739a0<r['rva']+r['size'] or r['rva']<=0x73ae0<r['rva']+r['size'] for r in contract['regions'])
out=dict(status='WHOLE_CODE_PREFLIGHT_PASS_TRUE_STORES_PENDING_COMPILE_AND_LINK',run=run,code_bytes=331,EH_bytes=0,direct025_code_credit=0,regions=rows,imports={'__imp__MultiByteToWideChar@24':dict(library='kernel32.lib',dll='KERNEL32.dll',reference_va=p.base+0xe5200)},identity='108-byte homologue9BC80 not selected: FontDrawTextA73BE0 calls739A0; proxyDraw73BC0 calls Consume73AE0. Both edges are observed, not renamed arbitrary matches.',stores=json.loads(Path('build/agent-textdraw/font-helper-stores-audit.json').read_text()),unimplemented_runtime_work='Caller DrawTextA229 and startupC4790 remain outside this lot. Startup actually allocates100001 bytes for pointer140AEC; no allocation or full lifecycle credit.',accepted_wide_parser_provider=wide['unit'])
Path('build/agent-textdraw/font-helpers-reviewed-manifest.json').write_text(json.dumps(out,indent=2)+'\n');Path('build/agent-textdraw/font-helpers-seeds.json').write_text(json.dumps([dict(unit=u,symbol=n,rva=r,size=z) for n,r,z in plan],indent=2)+'\n');print('PASS331 whole bytes,18 bindings; BSS40012 pending actual owners compilation/link')
