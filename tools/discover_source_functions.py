"""Discover full-function candidates, never accept inferred relocation targets."""
import argparse,json,struct
from collections import defaultdict
from pathlib import Path
from binary import COFF,PE,sha,u32
from capstone import Cs,CS_ARCH_X86,CS_MODE_32
from capstone.x86_const import X86_OP_MEM,X86_REG_FS
from verify import ROOT

def absolute_sites(raw, fixes):
 """FS:__except_list is absolute zero, not an image-base relocation."""
 sites=[]
 for f in fixes:
  if f['kind'] != 6:continue
  at=f['offset']
  if f['symbol']['name']=='__except_list':
   decoder=Cs(CS_ARCH_X86,CS_MODE_32);decoder.detail=True
   instruction=next((i for i in decoder.disasm(raw,0) if i.address<=at<i.address+i.size),None)
   if (instruction is None or u32(raw,at)!=0 or
       not any(o.type==X86_OP_MEM and o.mem.segment==X86_REG_FS and o.mem.disp==0 for o in instruction.operands)):
    raise ValueError('invalid FS pseudo-symbol operand')
  else:sites.append(at)
 return sorted(sites)

def main():
 p=argparse.ArgumentParser();p.add_argument('--run',required=True);p.add_argument('--units',required=True);a=p.parse_args()
 ref=PE(ROOT/'private/samp.dll');inv=json.loads((ROOT/'config/checkpoint31/inventory.json').read_text());sizes=defaultdict(list)
 for f in inv['functions']:
  if len(f['chunks'])==1:
   c=f['chunks'][0];sizes[c['size']].append((f['rva'],ref.read(c['rva'],c['size'])))
 rows=[]
 for unit in a.units.split(','):
  obj=COFF(ROOT/'build'/a.run/(unit+'.obj'))
  for name,ss in obj.names.items():
   if not any(s['section']>0 and s['type']==32 and s['storage']==2 and s['value']==0 for s in ss):continue
   try:raw,fixes=obj.function(name)
   except ValueError:continue
   image_sites=absolute_sites(raw,fixes)
   hits=[]
   for rva,expected in sizes[len(raw)]:
    output=bytearray(raw);targets={};consistent=True
    if image_sites!=sorted(p-rva for p in ref.relocations if rva<=p<rva+len(raw)):continue
    for f in fixes:
     at=f['offset'];target=u32(expected,at)
     if f['symbol']['name']=='__except_list' and target!=0:consistent=False;break
     if f['kind']==20:target=(target+ref.base+rva+at+4)&0xffffffff
     target=(target-u32(raw,at))&0xffffffff;symbol=f['symbol']['name']
     if symbol in targets and targets[symbol]!=target:consistent=False;break
     targets[symbol]=target;value=target+u32(raw,at)
     if f['kind']==20:value-=ref.base+rva+at+4
     struct.pack_into('<I',output,at,value&0xffffffff)
    if consistent and bytes(output)==expected:hits.append(dict(rva=rva,inferred_targets=targets))
   rows.append(dict(unit=unit,symbol=name,size=len(raw),object_sha256=sha(obj.data),candidates=hits))
 result=dict(status='DISCOVERY_ONLY_NOT_ACCEPTED',run=a.run,reference_sha256=sha(ref.data),rows=rows)
 (ROOT/'build'/a.run/'discovery.json').write_text(json.dumps(result,indent=2)+'\n')
 for r in rows:
  if r['candidates']:print(r['unit'],r['symbol'],r['size'],[hex(h['rva']) for h in r['candidates']])
 print('compiled',len(rows),'candidate functions',sum(bool(r['candidates']) for r in rows))
if __name__=='__main__':main()
