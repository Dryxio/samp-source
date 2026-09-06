import sys,json,copy
from pathlib import Path
sys.path.insert(0,'tools')
from verify_actor_closure import Gate
from binary import u32
run='cp32-selection-model-motion-linked3'
g=Gate(run,contract_path='config/checkpoint32/selection-model-motion-contract.json')
Path('build/selection-model-motion-associative-bindings.json').write_text(json.dumps(g.associative_bindings,indent=2)+'\n')
r=next(r for r in g.regions if r['unit']=='closure_object_editor_attached' and r['anchor'].startswith('__ehhandler$?FindPlayerPed'))
assert g.discarded_associative_owner(r)
obj=g.objects[r['unit']];saved=obj.data;ptr=u32(saved,8);data=bytearray(saved)
for index,s in obj.symbols.items():
 if s['section']==r['section'] and s['storage']==3 and s['type']==0 and s['name']==obj.sections[r['section']-1]['name']:
  data[ptr+(index+1)*18+14]=2
obj.data=bytes(data);assert g.discarded_associative_owner(r) is None
try:g.bind();raise AssertionError('non-associative section accepted')
except ValueError as e:assert 'object-local MAP symbol' in str(e)
obj.data=saved
parent='?FindPlayerPed@CGame@@QAEPAVCPlayerPed@@XZ';original=g.maps[parent];g.maps[parent]=[(original[0][0],'unknown-source.obj')]
assert g.discarded_associative_owner(r) is None;g.maps[parent]=original
source=next(q for q in g.regions if q['unit']==r['unit'] and q['anchor']==parent);digest=source['sha256'];source['sha256']='0'*64
assert g.discarded_associative_owner(r) is None;source['sha256']=digest
g.bind();at=r['linked_va']-g.linked.base
sec=next(s for s in g.linked.sections if s['rva']<=at<s['rva']+s['size']);raw=sec['bytes'];changed=bytearray(raw);changed[at-sec['rva']]^=1;sec['bytes']=bytes(changed)
try:g.verify();raise AssertionError('altered actual shared EH accepted')
except ValueError as e:assert 'full region differs' in str(e)
sec['bytes']=raw
rows=[dict(case=x,result='REJECTED') for x in ('non-associative section with missing local MAP owner','selected parent not owned by linked source','different original parent identity','altered actual selected EH byte')]
Path('build/associative-binding-controls.json').write_text(json.dumps(rows,indent=2)+'\n');print(json.dumps(rows))
