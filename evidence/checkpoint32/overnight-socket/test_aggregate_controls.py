import sys,json,copy
from pathlib import Path
sys.path[:0]=['tools','build/overnight']
from aggregate_support import aggregate
from integration_pipeline import Rejected
read=lambda p:json.loads(Path(p).read_text())
base=read('config/checkpoint32/overnight-wrappers-one-contract.json')
plan=read('evidence/checkpoint32/overnight-network/socket-storage-migration-plan.json')
review=read('evidence/checkpoint32/overnight-network/socket-layout-whole-block-source-review.json')
expected={}
for r in review['regions']:
 r=dict(r,unit='eval_store_socket_instance');expected[(r['unit'],r['rva'],r['size'])]=r
out=aggregate(base,plan,expected)
assert sum(r['size'] for r in out['regions'] if r['kind']=='code')==sum(r['size'] for r in base['regions'] if r['kind']=='code')
checks={'positive_entire_aggregate_zero_code_credit':'PASS'}
def reject(name,change):
 b,p,e=copy.deepcopy(base),copy.deepcopy(plan),copy.deepcopy(expected)
 change(b,p,e)
 try:aggregate(b,p,e)
 except (Rejected,KeyError,AssertionError):checks[name]='REJECTED';return
 raise AssertionError('Accepted bad case '+name)
reject('missing_global',lambda b,p,e:p['regions'].pop())
reject('shift_old_global',lambda b,p,e:p['regions'][-1].update(old_bytes_offset_in_new=1))
reject('truncated_old_global',lambda b,p,e:p['regions'][-1].update(old_size=397))
reject('wrong_old_identity',lambda b,p,e:p['regions'][-1].update(old_anchor='fake'))
reject('buffer_short',lambda b,p,e:p['aggregate_members'][2].update(logical_size=32767))
reject('buffer_long',lambda b,p,e:p['aggregate_members'][2].update(logical_size=32769))
reject('overlap_member',lambda b,p,e:p['aggregate_members'][2].update(offset=399))
reject('wrong_complete_size',lambda b,p,e:p.update(whole_object_size=33169))
reject('changed_reference_hash',lambda b,p,e:next(r for r in e.values() if r['kind']=='zero').update(sha256='0'*64))
Path('build/overnight/aggregate-negative-controls.json').write_text(json.dumps(checks,indent=2)+'\n')
print(json.dumps(checks,indent=2))
