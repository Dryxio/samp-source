import sys,json,struct,hashlib
from pathlib import Path
sys.path.insert(0,'tools')
from binary import PE,COFF,u32
from draft_actor_contract import weak_aliases
from capstone import Cs,CS_ARCH_X86,CS_MODE_32
p=Path('build/trial30c/network-agent');m=json.load(open(p/'graph-owner-reviewed-manifest.json'));c=json.load(open('config/checkpoint32/trial30b-two-contract.json'));idx=json.load(open('build/trial30c/symbol-index.json'))['symbols'];o=COFF(p/'graph-owner-discovery/closure_vendor_graph_algorithms.obj');pe=PE('private/samp.dll');d=Cs(CS_ARCH_X86,CS_MODE_32)
old=[r for r in c['regions'] if r['unit']=='closure_vendor_graph_algorithms'];checks=[]
localmap={}
# Establish object-local label rename solely from anchored complete EH sections.
for r in old:
 if r['anchor'].startswith('$'):continue
 sy=next(s for s in o.names[r['anchor']] if s['section']>0);sec=o.sections[sy['section']-1]
 assert sec['reloc_count']==len(r['fixups'])
 for j,f in enumerate(r['fixups']):
  off,kidx,k=struct.unpack_from('<IIH',o.data,sec['reloc_ptr']+10*j);n=o.symbols[kidx]['name']
  if f['symbol'].startswith('$'):
   assert n.startswith('$');assert (off,k)==(f['offset'],f['kind'])
   assert f['symbol'] not in localmap or localmap[f['symbol']]==n
   localmap[f['symbol']]=n
secmap={}
for r in old:
 sy=next(s for s in o.names[localmap.get(r['anchor'],r['anchor'])] if s['section']>0);secmap[sy['section']]=r['rva']
for r in old:
 ss=[s for s in o.names.get(localmap.get(r['anchor'],r['anchor']),[]) if s['section']>0];assert len(ss)==1,(r['anchor'],ss)
 sy=ss[0];sec=o.sections[sy['section']-1];assert sec['size']==r['size'] and sy['value']==r['anchor_offset'],(r,sec['size'],sy)
 raw=bytearray(sec['bytes']);assert sec['reloc_count']==len(r['fixups'])
 for j,f in enumerate(r['fixups']):
  off,kidx,k=struct.unpack_from('<IIH',o.data,sec['reloc_ptr']+10*j);n=o.symbols[kidx]['name'];assert (off,k)==(f['offset'],f['kind'])
  if f['symbol'].startswith('$'):
   assert n.startswith('$');ns=o.symbols[kidx];assert pe.base+secmap[ns['section']]+ns['value']==f['reference_va'],(n,ns,f)
  else:assert n==f['symbol'],(n,f)
  va=f['reference_va']+u32(raw,off)
  if k==20:va-=pe.base+r['rva']+off+4
  struct.pack_into('<I',raw,off,va&0xffffffff)
 assert bytes(raw)==pe.read(r['rva'],r['size']),r['anchor']
 checks.append(dict(anchor=r['anchor'],rva=r['rva'],size=r['size'],whole_equal=True))
# Three specialized declarations: real accepted implementations have no calls,
# imports, throws, or EH; whole bodies are bounded scalar memory operations.
no_throw={};rs={r['anchor']:r for r in c['regions']}
def no_throw_tree(n,seen=None):
 seen=set() if seen is None else seen
 assert n not in seen,n
 seen.add(n);r=rs[n];assert r['kind']=='code';code=pe.read(r['rva'],r['size']);ins=list(d.disasm(code,pe.base+r['rva']));children={}
 assert not any(i.mnemonic in ('int','int3','syscall') for i in ins)
 for i in ins:
  if i.mnemonic=='call' or i.mnemonic=='jmp' and not (pe.base+r['rva']<=int(i.op_str,16)<pe.base+r['rva']+r['size']):
   fs=[f for f in r['fixups'] if i.address-pe.base-r['rva']<=f['offset']<i.address-pe.base-r['rva']+i.size]
   assert len(fs)==1 and fs[0]['kind']==20,(n,i.op_str)
   f=fs[0];assert f['symbol'] in rs and f['reference_va']==pe.base+rs[f['symbol']]['rva'];children[f['symbol']]=no_throw_tree(f['symbol'],seen.copy())
 return dict(rva=r['rva'],size=r['size'],disassembly=[i.mnemonic+' '+i.op_str for i in ins],accepted_direct_children=children)
for n,rows in idx.items():
 if ('?GetKeyAtIndex@?$Map@UPlayerIdAndGroupId@ConnectionGraph' in n) or n.startswith('?Pop@?$Queue@UPlayerIdAndGroupId@ConnectionGraph'):
  row=next(r for r in rows if r['accepted']);ar=next(a for a in row['accepted'] if a['kind']=='code');instructions=list(d.disasm(pe.read(ar['rva'],ar['size']),pe.base+ar['rva']));tree=no_throw_tree(n)
  no_throw[n]=dict(owner=row['unit'],rva=ar['rva'],size=ar['size'],complete_call_tree=tree)
assert len(no_throw)==3
rs={r['anchor']:r for r in c['regions']};ext={}
for n,v in m['external_targets'].items():
 if n.startswith('$') or n.startswith('__ehhandler'):continue
 if n in rs:assert v==pe.base+rs[n]['rva'];ext[n]=dict(kind='accepted provider',owner=rs[n]['unit'],rva=rs[n]['rva'])
 elif n in c['externals']:assert v==c['externals'][n]['reference_va'];ext[n]=dict(kind='existing reviewed external',provider=c['externals'][n])
 else:raise AssertionError(n)
for r in m['regions']:
 assert {b['site_rva'] for b in r['bindings'] if b['kind']==6 and b['symbol']!='__except_list'}=={a for a in pe.relocations if r['rva']<=a<r['rva']+r['size']}
report=dict(status='PASS_SCOPE_AND_OWNER_AUDIT_NOT_ACCEPTANCE',source_proposal='eval_vendor_graph_nothrow_trial.cpp',real_owner='closure_vendor_graph_algorithms',new_unique=1255,old_owner_regions=checks,throw_declaration_evidence=no_throw,external_target_identity=ext,weak_aliases=weak_aliases(o),layout='Real Node7,WeightedGraph52,GraphAdjacency24,GraphTree19,GraphHeap12 unchanged static asserts; no storage/global/CRT additions.',identity_source='Old source graph-algorithms-proposal matched three high Router algorithms, full call graph remains; only Dijkstra1255 including85EH selected now. Each used real helper is already accepted and agrees originaltarget.',source025_direct_bytes=0,prepared_before_trial=False,adaptation='Fresh accurate throw() specialization declarations recover original optimizer knowledge for three real nonthrowing providers without emitting duplicate definitions.',excluded='ShortestPath1010 vs1026 remains mismatched; SpanningTree1059 depends on it. Neither counted.')
(p/'graph-actual-owner-scope-audit.json').write_text(json.dumps(report,indent=2)+'\n');print('PASS',len(checks),'old regions',len(no_throw),'nonthrowing providers',1255,'new proposed')
