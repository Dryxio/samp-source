"""Real Gate method: immutable linked artifacts, copied in-memory fault cases."""
from pathlib import Path
import argparse,copy,json,struct,sys,hashlib
sys.path.insert(0,'tools')
from verify_actor_closure import Gate
ap=argparse.ArgumentParser();ap.add_argument('--run',required=True);ap.add_argument('--contract',required=True);args=ap.parse_args()
g=Gate(args.run,fresh=False,contract_path=args.contract)
r=next(r for r in g.regions if r['rva']==0xfe008 and r['size']==8)
assert r['unit']=='closure_vendor_network_id_generator'
assert g.bind_crt_initializer(r)
p=next(x for x in g.crt_initializer_bindings if x['unit']==r['unit'] and x['original_rva']==r['rva'])
slot=p['linked_rva'];begin,end=p['sentinels'];targets=[x['provider_linked_va'] for x in p['providers']]
assert len(targets)==2 and targets[0]!=targets[1]
results=[dict(name='actual_positive8',result='PASS',proof=p)]
# Preserve established scalar behavior on these same real linked inputs.
four=next(q for q in g.regions if q['unit']=='closure_store_ime_candidate_state' and q['anchor']=='_$S7')
assert four['size']==4 and g.bind_crt_initializer(four)
results.append(dict(name='actual_positive4',result='PASS'))
def overlay(pe,replacements):
 old=pe.read
 def read(at,size):
  out=bytearray(old(at,size))
  for address,value in replacements.items():
   for i,v in enumerate(struct.pack('<I',value)):
    if at<=address+i<at+size:out[address+i-at]=v
  return bytes(out)
 pe.read=read

def clone(linked=None,reference=None):
 h=copy.copy(g);h.linked=copy.copy(g.linked);h.reference=copy.copy(g.reference)
 h.linked.relocations=set(g.linked.relocations);h.reference.relocations=set(g.reference.relocations)
 h.maps=copy.deepcopy(g.maps);h.crt_initializer_bindings=copy.deepcopy(g.crt_initializer_bindings)
 h.actual_sections=dict(g.actual_sections);h.by_section=copy.deepcopy(g.by_section)
 if linked:overlay(h.linked,linked)
 if reference:overlay(h.reference,reference)
 return h

def reject(name,h,expected):
 try:h.bind_crt_initializer(dict(r))
 except ValueError as e:
  assert expected in str(e),(name,str(e));results.append(dict(name=name,result='REJECTED_AS_REQUIRED',reason=str(e)))
 else:raise AssertionError(name+' unexpectedly accepted')
reject('wrong_second_linked_pointer',clone({slot+4:targets[1]+4}),'missing or ambiguous CRT initializer slot block')
h=clone();h.linked.relocations.remove(slot+4)
reject('missing_second_linked_relocation',h,'missing linked CRT initializer PE relocation')
reject('reversed_linked_sequence',clone({slot:targets[1],slot+4:targets[0]}),'missing or ambiguous CRT initializer slot block')
spare=next(a for a in range(begin+4,end-7,4) if a+8<=slot or a>=slot+8)
reject('duplicate_complete_block',clone({spare:targets[0],spare+4:targets[1]}),'missing or ambiguous CRT initializer slot block')
h=clone();h.maps['___xc_z']=[(h.linked.base+begin+8,'LIBCMT:CRT0INIT.OBJ')]
reject('false_sentinel_bound',h,'invalid CRT initializer bounds')
h=clone();original_provider=h.provider
second_symbol=p['providers'][1]['provider']
def nonlocal_provider(unit,symbol):
 target,offset=original_provider(unit,symbol)
 if not isinstance(target,str) and target['anchor']==second_symbol:
  target=dict(target,unit='not_the_initializer_owner')
 return target,offset
h.provider=nonlocal_provider
reject('second_provider_not_local',h,'CRT initializer lacks a complete local provider')
reject('wrong_second_original_pointer',clone(reference={r['rva']+4:g.reference.base+0xe4294}),'wrong original CRT initializer target or relocation')
h=clone();h.reference.relocations.remove(r['rva']+4)
reject('missing_second_original_relocation',h,'wrong original CRT initializer target or relocation')
# Recheck original objects and complete linked inputs after every overlay test.
assert g.bind_crt_initializer(r) and g.bind_crt_initializer(four)
out=dict(status='PASS_REAL_GATE_TWO_POSITIVE_EIGHT_NEGATIVE_CONTROLS',run=args.run,contract=args.contract,controls=results,mutation_scope='Copied gate state and in-memory PE read overlays only; no source, contract, object, DLL or MAP writes.',new_credit=0,verifier_sha256=hashlib.sha256(Path('tools/verify_actor_closure.py').read_bytes()).hexdigest(),test_sha256=hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),linked_sha256=hashlib.sha256((Path('build')/args.run/'closure.dll').read_bytes()).hexdigest())
Path('build/agent-textdraw/crt-initializer-block-controls.json').write_text(json.dumps(out,indent=2)+'\n')
print(json.dumps(out,indent=2))
