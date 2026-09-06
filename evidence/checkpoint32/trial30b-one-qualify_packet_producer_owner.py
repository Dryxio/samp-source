from pathlib import Path
import json,sys,struct,hashlib
sys.path.insert(0,'tools')
from binary import PE,COFF,u32
from draft_actor_contract import weak_aliases
p=PE(Path('private/samp.dll'));unit='eval_vendor_rakpeer_send_handshake';op=Path('build/cp32-trial30b-one-ob1')/(unit+'.obj');c=COFF(op);name='?AddPacketToProducer@RakPeer@@IAEXPAUPacket@@@Z';rv=0x3b090
raw,fix=c.function(name);sy=next(s for s in c.names[name] if s['section']>0);assert sy['value']==0 and len(raw)==31;ref=p.read(rv,len(raw));b=bytearray(raw);bindings=[]
proof=json.load(open('evidence/checkpoint32/trial30-final-acceptance.json'));by={r['anchor']:r for r in proof['regions']}
for f in fix:
 n=f['symbol']['name'];r=by[n];target=r['rva']+p.base;at=f['offset'];assert f['kind']==20;actual=(u32(ref,at)+p.base+rv+at+4)&0xffffffff;assert target+u32(raw,at)==actual
 struct.pack_into('<I',b,at,(actual-p.base-rv-at-4)&0xffffffff);bindings.append(dict(symbol=n,site_rva=rv+at,target_va=actual,kind=20))
assert bytes(b)==ref and not any(rv<=x<rv+31 for x in p.relocations)
source=Path('client/saco')/(unit+'.cpp');out=dict(status='PASS_EXISTING_EMITTED_SOURCE_PROVIDER_NEWLY_QUALIFIED',roots={name:rv},seeds={unit:{name:rv}},regions=[dict(unit=unit,section=sy['section'],rva=rv,size=31,kind='.text',sha256=hashlib.sha256(ref).hexdigest(),whole_bytes_equal=True,bindings=bindings)],bindings_checked=2,new_unique_code=31,source025_direct_bytes=0,source_files=[dict(path=str(source),sha256=hashlib.sha256(source.read_bytes()).hexdigest())],objects={unit:hashlib.sha256(op.read_bytes()).hexdigest()},actual_weak_aliases={unit:weak_aliases(c)},note='Whole31 provider emitted by existing real SendImmediate TU; no source changes or duplicateowner. Existing lowerWriteLock64 andWriteUnlock20 alreadyaccepted and actualtargets verified. No aliases or newallocations.')
Path('build/agent-network-b/packet-producer-owner-reviewed-manifest.json').write_text(json.dumps(out,indent=2)+'\n');Path('build/agent-network-b/packet-producer-owner-seeds.json').write_text(json.dumps(out['seeds'],indent=2)+'\n');print('PASS31')
