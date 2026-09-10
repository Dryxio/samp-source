"""Preserve complete existing globals inside a reviewed complete source BSS section."""
import copy
from integration_pipeline import ROOT,require
from binary import PE,sha
from verify_actor_closure import initial_bytes

def aggregate(base,plan,expected):
    out=copy.deepcopy(base);pe=PE(ROOT/'private/samp.dll');items=plan['regions'];zero=[m for m in items if m['old_kind']=='zero']
    require(len(zero)==3,'Expected exhaustive three-global aggregation')
    newkeys={(m['new_unit'],m['new_section_rva'],m['new_section_size']) for m in zero};require(len(newkeys)==1,'Multiple aggregate destinations')
    key=next(iter(newkeys));e=expected[key]
    require(e['kind']=='zero' and e['size']==plan['whole_object_size']==33170,'Not complete reviewed socket BSS')
    raw=initial_bytes(pe,e['rva'],e['size']);require(raw==bytes(e['size']) and sha(raw)==e['sha256'],'Original aggregate not complete zero block')
    members=plan['aggregate_members'];cursor=0
    for m in members:
        require(m['offset']==cursor,'Aggregate member layout gap/overlap')
        cursor+=m.get('logical_size',m.get('size',0))
    require(cursor==e['size'],'Incomplete aggregate members')
    require([m.get('logical_size',m.get('size')) for m in members]==[398,2,32768,1,1],'Unreviewed socket member types or extent')
    remove=[];prototype=None
    for m in zero:
        found=[r for r in out['regions'] if (r['unit'],r['rva'],r['size'],r['kind'])==(m['old_unit'],m['old_rva'],m['old_size'],m['old_kind'])]
        require(len(found)==1,'Missing original entire global')
        r=found[0];require(r['anchor']==m['old_anchor'] and not r['fixups'],'Old global identity changed')
        offset=m['old_bytes_offset_in_new'];require(e['rva']+offset==r['rva'] and offset+r['size']<=e['size'],'Old global relocated or truncated')
        require(sha(raw[offset:offset+r['size']])==r['sha256'],'Old global bytes changed')
        remove.append(r);prototype=r
    for r in remove:out['regions'].remove(r)
    r=copy.deepcopy(prototype)
    r.update(unit=e['unit'],section=e['section'],anchor=e['anchor'],anchor_offset=e.get('anchor_offset',0),offset=0,rva=e['rva'],size=e['size'],kind='zero',sha256=e['sha256'],fixups=[],implementation='Complete reviewed socket BSS aggregation preserving all three old global byte ranges; new buffer and compiler padding earn zero code credit.')
    out['regions'].append(r)
    return out
