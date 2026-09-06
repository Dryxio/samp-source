import copy,json,sys,struct
from pathlib import Path
sys.path.insert(0,'tools');from binary import sha
from verify_actor_closure import Gate
run='cp32-console-font-linked1';g=Gate(run,contract_path='config/checkpoint32/console-font-contract.json');g.contract=copy.deepcopy(g.contract);name='___mb_cur_max';expected=struct.pack('<III',1,0x2e,1);g.contract['externals'][name]=dict(kind='crt-data',reference_va=g.reference.base+0x117668,size=12,sha256=sha(expected),library='libcmt.lib',member='nlsdata1.obj')
assert g.external(name,False)==g.reference.base+0x117668;actual=g.external(name,True);results=[dict(name='actual_pinned_block_both_images',status='PASS')]
def clone():
 h=copy.copy(g);h.contract=copy.deepcopy(g.contract);h.maps=copy.deepcopy(g.maps);h.reference=copy.copy(g.reference);h.linked=copy.copy(g.linked);h.reference.relocations=set(g.reference.relocations);h.linked.relocations=set(g.linked.relocations);return h
def reject(h,n,reason):
 try:h.external(name,True)
 except ValueError as e:assert reason in str(e);results.append(dict(name=n,status='REJECTED_AS_REQUIRED',reason=str(e)))
 else:raise AssertionError('accepted fault '+n)
def overlay(pe,at):
 read=pe.read
 def changed(r,size):
  raw=bytearray(read(r,size))
  if r<=at<r+size:raw[at-r]^=1
  return bytes(raw)
 pe.read=changed
h=clone();h.contract['externals'][name]['size']=4;reject(h,'truncated_scalar','unreviewed CRT data extent')
h=clone();h.maps[name]=[(actual,'evil.obj')];reject(h,'wrong_archive_provider','wrong CRT data provider')
h=clone();h.maps['___decimal_point']=[(actual+8,'LIBCMT:nlsdata1.obj')];reject(h,'wrong_neighbor_offset','wrong CRT data provider')
h=clone();overlay(h.linked,actual-h.linked.base+8);reject(h,'wrong_last_linked_object','wrong complete CRT data bytes')
h=clone();overlay(h.reference,0x11766c);reject(h,'wrong_original_decimal','wrong complete CRT data bytes')
h=clone();h.linked.relocations.add(actual-h.linked.base+4);reject(h,'unexpected_linked_relocation','unexpected CRT data relocation')
h=clone();h.reference.relocations.add(0x117668);reject(h,'unexpected_original_relocation','unexpected CRT data relocation')
assert g.external(name,True)==actual
out=dict(status='PASS_REAL_CRT_DATA_GATE_ONE_POSITIVE_SEVEN_NEGATIVE',run=run,controls=results,verifier_sha256=sha(Path('tools/verify_actor_closure.py').read_bytes()),test_sha256=sha(Path(__file__).read_bytes()),linked_sha256=sha(g.linked.data),mutation_scope='Copied in-memory gate states/read overlays only. No artifact mutation.',new_credit=0)
Path('build/crt-locale-data-controls.json').write_text(json.dumps(out,indent=2)+'\n');print(out['status'])
