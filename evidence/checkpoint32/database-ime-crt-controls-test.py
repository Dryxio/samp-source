"""Exercise actual Gate.bind_crt_initializer on real linked inputs and in-memory faults."""
from pathlib import Path
import copy,json,sys,struct,hashlib
sys.path.insert(0,'tools')
from verify_actor_closure import Gate
from binary import u32
run='cp32-database-ime-linked2';contract='config/checkpoint32/database-ime-contract.json'
g=Gate(run,fresh=False,contract_path=contract)
r=next(r for r in g.regions if r['unit']=='closure_store_ime_candidate_state' and r['anchor']=='_$S7')
assert g.bind_crt_initializer(r) is True
proof=next(x for x in g.crt_initializer_bindings if x['unit']==r['unit']);slot=proof['linked_rva'];target=proof['provider_linked_va'];begin,end=proof['sentinels'];results=[dict(name='actual_positive',result='PASS',linked_rva=slot,target_va=target)]
def mutated_gate(replacements=None,remove_relocation=None):
 h=copy.copy(g);h.linked=copy.copy(g.linked);h.linked.relocations=set(g.linked.relocations);h.maps=dict(g.maps);h.crt_initializer_bindings=list(g.crt_initializer_bindings)
 if remove_relocation is not None:h.linked.relocations.remove(remove_relocation)
 if replacements:
  original=g.linked.read
  def read(at,size):
   out=bytearray(original(at,size))
   for address,value in replacements.items():
    data=struct.pack('<I',value)
    for i,v in enumerate(data):
     if at<=address+i<at+size:out[address+i-at]=v
   return bytes(out)
  h.linked.read=read
 return h
def reject(name,h,expected):
 try:h.bind_crt_initializer(r)
 except ValueError as e:
  assert expected in str(e),(name,str(e));results.append(dict(name=name,result='REJECTED_AS_REQUIRED',reason=str(e)))
 else:raise AssertionError(name+' unexpectedly accepted')
spare=next(a for a in range(begin+4,end,4) if a!=slot)
reject('duplicate_true_target',mutated_gate({spare:target}),'ambiguous CRT initializer slot')
reject('wrong_unique_target',mutated_gate({slot:target+4}),'missing or ambiguous CRT initializer slot')
reject('missing_linked_PE_relocation',mutated_gate(remove_relocation=slot),'missing linked CRT initializer PE relocation')
h=mutated_gate();h.maps['___xc_a']=[(g.maps['___xc_a'][0][0],'untrusted.obj')]
reject('wrong_CRT_boundary_owner',h,'invalid pinned CRT sentinel owner')
reject('nonzero_null_sentinel',mutated_gate({begin:1}),'invalid CRT sentinel section or contents')
# Negative controls never write or alter the original artifacts.
assert g.bind_crt_initializer(r) is True
out=dict(status='PASS_REAL_GATE_METHOD_POSITIVE_AND_FIVE_NEGATIVE_CONTROLS',run=run,contract=contract,method='Gate.bind_crt_initializer',verifier_sha256=hashlib.sha256(Path('tools/verify_actor_closure.py').read_bytes()).hexdigest(),test_sha256=hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),linked_sha256=hashlib.sha256((Path('build')/run/'closure.dll').read_bytes()).hexdigest(),controls=results,mutation_scope='Only shallow gate copies and in-memory PE read overlays / copied relocation sets / copied MAP entries; no artifact/source/contract files altered.',new_credit=0)
Path('build/agent-independent/crt-initializer-binding-controls.json').write_text(json.dumps(out,indent=2)+'\n');print(json.dumps(results,indent=2))
