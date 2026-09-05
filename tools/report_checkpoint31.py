"""Produce measurable coverage, library dependency inventory and next-work list."""
from collections import Counter,defaultdict
from binary import sha
from verify import ROOT
from verify_checkpoint31 import read_json


def report(gate,result):
    accepted={r['symbol']:r for r in result['regions']}
    mapped={f['name']:f for f in gate.inventory['functions']}
    definitions=defaultdict(set);undefined=defaultdict(set);function_rows=[]
    for filename,obj in gate.objects.items():
        for symbol in obj.symbols.values():
            name=symbol['name']
            if symbol['storage']!=2:continue
            if symbol['section']>0 or symbol['section']==-1 or symbol['value']:
                definitions[name].add(filename)
            else:undefined[name].add(filename)
        for name in obj.names:
            try:raw,relocations=obj.function(name)
            except ValueError:continue
            if name in accepted:
                status='ACCEPTED_RAW_LINKED_EXACT' if filename==accepted[name]['object'] else 'DUPLICATE_SYMBOL_NOT_SEPARATELY_ACCEPTED'
                rva=accepted[name]['rva']
            elif name in mapped:status='IDENTIFIED_NOT_ACCEPTED';rva=mapped[name]['rva']
            else:status='SOURCE_AVAILABLE_UNMAPPED';rva=None
            function_rows.append(dict(object=filename,symbol=name,bytes=len(raw),fixups=len(relocations),rva=rva,status=status))
    unresolved=[]
    for name,users in sorted(undefined.items()):
        if name in definitions:continue
        game=any(x in name for x in ('ID3DXFontHook','GetFontFace','GetFontWeight','GetUIFontSize','pGame'))
        unresolved.append(dict(symbol=name,users=sorted(users),
                               category='GAME_BOUNDARY_NOT_IMPLEMENTED' if game else 'SDK_CRT_OR_SYSTEM_AT_ARCHIVE_BOUNDARY',
                               linked_symbol_present=name in gate.maps))
    union=set();scope_sets={}
    cp1=read_json(ROOT/'config/manifest.json')['functions']
    cp2=read_json(ROOT/'config/checkpoint2/regions.json')['regions']
    scopes={'checkpoint1':[(r['rva'],r['size']) for r in cp1],
            'checkpoint2':[(r['rva'],r['code_size']) for r in cp2],
            'checkpoint31':[(r['rva'],r['size']) for r in result['regions']]}
    for name,ranges in scopes.items():
        span=set()
        for rva,size in ranges:span.update(range(rva,rva+size))
        scope_sets[name]=span;union.update(span)
    text=next(s for s in gate.reference.sections if s['name']=='.text')
    need_text={r for r in union if text['rva']<=r<text['rva']+text['virtual_size']}
    incoming=Counter(e['target'] for e in gate.inventory['edges'] if e.get('target_kind')=='direct')
    known={r['rva']:r['symbol'] for r in result['regions']}
    pending=[]
    for f in gate.inventory['functions']:
        if f['rva'] in known:continue
        if all(all(byte in union for byte in range(c['rva'],c['rva']+c['size'])) for c in f['chunks']):continue
        pending.append(dict(rva=f['rva'],name=f['name'],classification=f['classification'],
                            incoming_direct_calls_or_jumps=incoming[f['rva']],
                            bytes=sum(c['size'] for c in f['chunks']),status='NOT_ACCEPTED'))
    pending.sort(key=lambda f:(-f['incoming_direct_calls_or_jumps'],-f['bytes'],f['rva']))
    return dict(reference_sha256=sha(gate.reference.data),function_hints=len(gate.hints),
                inventory_status='Names, boundaries and graph are analysis hints; none are coverage by themselves',
                source_units=len(gate.objects),source_function_comdats=len(function_rows),
                function_rows=function_rows,archive_external_dependencies=unresolved,
                coverage=dict(cp31_regions=len(result['regions']),cp31_code_bytes=len(scope_sets['checkpoint31']),
                              cp1_cp2_cp31_union_code_bytes=len(union),text_bytes=text['virtual_size'],
                              text_coverage_percent=round(100*len(need_text)/text['virtual_size'],6),
                              status='UNION_OF_EXACT_REGIONS_NOT_WHOLE_LIBRARY_CERTIFICATION'),
                priorities=pending[:100],
                additional_constraints=['Game-specific font/window hooks are fail-fast test boundaries, not production implementations',
                                        'Library regions requiring relocation contracts are not accepted by this raw-only batch',
                                        'BASS and D3DX are external dependencies identified from reference IAT; not reconstructed libraries',
                                        'Indirect calls in accepted caller bodies do not certify their dynamic target implementations'])
