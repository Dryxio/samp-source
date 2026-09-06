from pathlib import Path
import sys,json,struct,hashlib
sys.path.insert(0,'tools')
from binary import PE,COFF,u32
p=PE(Path('private/samp.dll'));import argparse
ap=argparse.ArgumentParser();ap.add_argument('--run',required=True);ap.add_argument('--baseline',required=True);args=ap.parse_args()
p=PE(Path('private/samp.dll'));run=Path('build')/args.run
rows=json.loads((run/'discovery-attached.json').read_text())['rows']
byname={r['symbol']:r for r in rows};roots={}
from draft_actor_contract import weak_aliases
rawobjects={u:COFF(run/(u+'.obj')) for u in {r['unit'] for r in rows}}
classes=['ConsoleServer','CommandParserInterface','LogCommandParser','TelnetTransport','PacketConsoleLogger']
import re
for r in rows:
 n=r['symbol']
 if any(re.match(r'^\?[^?@][^@]*@'+cl+'@@',n) or n.startswith(('??0'+cl+'@@','??1'+cl+'@@')) for cl in classes):
  if len(r['candidates'])==1:roots[n]=r['candidates'][0]['rva']
# Log constructor supplies the true base constructor identity, then real VFT entries identify homologues.
log=byname['??0LogCommandParser@@QAE@XZ']['candidates'][0]
base='??0CommandParserInterface@@QAE@XZ';base_rva=log['inferred_targets'][base]-p.base;roots[base]=base_rva
vft_anchors={}
for cl in classes[1:]:
 row=byname['??0'+cl+'@@QAE@XZ'];rv=roots[row['symbol']];cand=next(c for c in row['candidates'] if c['rva']==rv);v='??_7'+cl+'@@6B@';vaddr=cand['inferred_targets'][v]-p.base;vft_anchors[v]=vaddr
 co=rawobjects[row['unit']];sy=next(s for s in co.names[v] if s['section']>0);sec=co.sections[sy['section']-1];aliases=weak_aliases(co)
 for j in range(sec['reloc_count']):
  off,idx,kind=struct.unpack_from('<IIH',co.data,sec['reloc_ptr']+10*j);n=co.symbols[idx]['name'];n=aliases.get(n,n);target=u32(p.read(vaddr+off,4),0)-p.base
  if n in byname:
   assert any(c['rva']==target for c in byname[n]['candidates']),(n,hex(target));roots[n]=target

def read_initialized(rva,size):
 try:return p.read(rva,size)
 except ValueError:
  sec=next(s for s in p.sections if s['rva']+s['size']<=rva and rva+size<=s['rva']+s['virtual_size'])
  assert sec['name']=='.data'
  return bytes(size)
objects={u:COFF(run/(u+'.obj')) for u in {r['unit'] for r in rows}}
# Compiler local labels and internal-linkage constants are object-local.
local_names={u:{s["name"] for s in c.symbols.values() if s and s["section"]>0 and s["storage"]==3 and s["type"]!=32 and not s["name"].startswith(".")} for u,c in objects.items()}
for unit,c in objects.items():
 for sym in c.symbols.values() if isinstance(c.symbols,dict) else c.symbols:
  if sym and (sym['name'].startswith('$') or sym['name'] in local_names[unit]):
   sym['name']=unit+'::'+sym['name']
 c.names={}
 for sym in c.symbols.values() if isinstance(c.symbols,dict) else c.symbols:
  if sym:c.names.setdefault(sym['name'],[]).append(sym)
ids={};selected={};todo=list(roots.items());sections={u:{} for u in objects};reasons={k:'source method semantics + original call graph / allocation / table owner' for k in roots}
def bind(name,va):
 if name in ids:assert ids[name]==va,(name,hex(ids[name]),hex(va))
 ids[name]=va
while todo:
 name,rva=todo.pop()
 if name in selected:assert selected[name]['rva']==rva;continue
 row=byname[name];cand=next(c for c in row['candidates'] if c['rva']==rva)
 selected[name]=dict(unit=row['unit'],rva=rva,size=row['size']);bind(name,p.base+rva)
 c=objects[row['unit']];sym=next(s for s in c.names[name] if s['section']>0 and s['type']==32);sections[row['unit']][sym['section']]=rva
 for n,va in cand['inferred_targets'].items():
  n=row['unit']+'::'+n if n.startswith('$') or n in local_names[row['unit']] else n
  bind(n,va)
  if n in byname and n not in selected:
   assert any(x['rva']==va-p.base for x in byname[n]['candidates']),(n,hex(va));todo.append((n,va-p.base));reasons[n]='actual relocation from '+name
# A true shared function may be emitted in multiple TUs; qualify every actual
# source-unit section used by its local relocations, not only the first symbol owner.
for unit,c in objects.items():
 for name,va in list(ids.items()):
  for sy in c.names.get(name,[]):
   if sy['section']>0 and sy['type']==32 and c.sections[sy['section']-1]['name']=='.text':
    sections[unit][sy['section']]=va-p.base-sy['value']
# EH section base anchored at known handler target, then unwind/FuncInfo local pointers.
for unit,c in objects.items():
 changed=True
 while changed:
  changed=False
  for n,va in list(ids.items()):
   for s in c.names.get(n,[]):
    if s['section']>0 and c.sections[s['section']-1]['name'] in ['.text$x','.xdata$x','.rdata','.data','.bss'] and s['section'] not in sections[unit]:
     sections[unit][s['section']]=va-p.base-s['value'];changed=True
  for si,rva in list(sections[unit].items()):
   sec=c.sections[si-1]
   if sec['name'] not in ['.text$x','.xdata$x','.rdata','.data','.bss']:continue
   ref=read_initialized(rva,sec['size'])
   for j in range(sec['reloc_count']):
    off,idx,k=struct.unpack_from('<IIH',c.data,sec['reloc_ptr']+10*j);sym=c.symbols[idx];va=u32(ref,off)
    if k==20:va=(va+p.base+rva+off+4)&0xffffffff
    va-=u32(sec['bytes'],off)
    if sym['name'] not in ids:ids[sym['name']]=va;changed=True
    else:assert ids[sym['name']]==va,(unit,sym['name'],hex(ids[sym['name']]),hex(va),hex(rva),si,[n for n,v in ids.items() if v==p.base+rva])
out=dict(status='REVIEWED_MAIN_SEEDS_AND_WHOLE_EH_BINDINGS_PREFLIGHT',run=str(run),roots=roots,seeds={u:{} for u in objects},regions=[],bindings_checked=0,external_targets={})
for name,row in selected.items():out['seeds'][row['unit']][name]=row['rva']
for unit,c in objects.items():
 for si,rva in sections[unit].items():
  sec=c.sections[si-1];raw=bytearray(bytes(sec['size']) if sec['uninitialized'] else sec['bytes']);ref=read_initialized(rva,len(raw));bs=[]
  for j in range(sec['reloc_count']):
   off,idx,k=struct.unpack_from('<IIH',c.data,sec['reloc_ptr']+10*j);name=c.symbols[idx]['name'];target=ids[name]+u32(raw,off);actual=u32(ref,off)
   if k==20:actual=(actual+p.base+rva+off+4)&0xffffffff;value=(target-(p.base+rva+off+4))&0xffffffff
   else:assert k==6;value=target
   assert target==actual,(name,hex(target),hex(actual));assert ((rva+off) in p.relocations)==(k==6 and name!='__except_list');struct.pack_into('<I',raw,off,value);bs.append(dict(symbol=name.split('::',1)[-1],site_rva=rva+off,target_va=target,kind=k))
  assert bytes(raw)==ref,(unit,si,rva)
  assert {b['site_rva'] for b in bs if b['kind']==6 and b['symbol']!='__except_list'}=={v for v in p.relocations if rva<=v<rva+len(raw)},(unit,si,'whole PE relocation set')
  out['regions'].append(dict(unit=unit,section=si,rva=rva,size=len(raw),kind=sec['name'],sha256=hashlib.sha256(ref).hexdigest(),whole_bytes_equal=True,bindings=bs));out['bindings_checked']+=len(bs)
for n,v in ids.items():
 if n not in selected:out['external_targets'][n]=v
out['identity_reasons']=reasons
from rank_uncovered_candidates import accepted_code
proof_path=args.baseline
proof=json.loads(Path(proof_path).read_text());covered=accepted_code(proof_path)
physical_code={a for r in out['regions'] if r['kind'].startswith('.text') for a in range(r['rva'],r['rva']+r['size'])}
inventory=json.loads(Path('config/checkpoint31/inventory.json').read_text())
original_code={a for f in inventory['functions'] for c in f['chunks'] for a in range(c['rva'],c['rva']+c['size'])}
code=physical_code & original_code
out['new_unique_code']=len(code-covered);out['already_covered_code']=len(code&covered);out['physical_code_compared']=len(physical_code);out['baseline']=len(covered);out['source025_direct_bytes']=0
existing={r['anchor']:r for r in proof['regions'] if r['kind']=='code'}
out['reuse_existing_owners']={}
for unit,seed in out['seeds'].items():
 for name in list(seed):
  if name in existing and not name.startswith('_$') and existing[name]['unit']!='closure_vendor_network_id_tree':
   r=existing[name];assert seed[name]==r['rva'],(name,seed[name],r)
   out['reuse_existing_owners'][name]=dict(unit=r['unit'],rva=r['rva'],size=r['size']);del seed[name]
out['excluded_unmatched']=[dict(symbol=r['symbol'],size=r['size']) for r in rows if not r['candidates']]
out['explicit_owner_transfer']='None; new TCP/RakSleep sole owners.'
out['unselected_method']='Unreferenced homologues are excluded; all Nat roots linked to actual calls, data, allocation, and VFT.'
out['source_files']=[dict(path=str(f),sha256=hashlib.sha256(f.read_bytes()).hexdigest()) for f in [Path('client/saco')/(u+'.cpp') for u in objects]]
out['objects']={unit:hashlib.sha256((run/(unit+'.obj')).read_bytes()).hexdigest() for unit in objects}
from draft_actor_contract import weak_aliases
out['actual_weak_aliases']={unit:weak_aliases(c) for unit,c in objects.items()}
Path('build/agent-independent/console-reviewed-manifest.json').write_text(json.dumps(out,indent=2)+'\n')
Path('build/agent-independent/console-seeds.json').write_text(json.dumps(out['seeds'],indent=2)+'\n')
print(len(selected),len(out['regions']),out['bindings_checked'],out['new_unique_code'],out['already_covered_code'])
