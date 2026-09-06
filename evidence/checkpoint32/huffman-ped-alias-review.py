import sys,json
from pathlib import Path
sys.path.insert(0,'tools')
from binary import PE,COFF,u32
from verify_checkpoint2 import map_symbols,symbol_address
run=sys.argv[1];directory=Path('build')/run
maps=map_symbols(directory/'closure.map');pe=PE(directory/'closure.dll');rows=[];calls=[]
work=[('closure_rw_parser_symbolic',json.load(open('build/agent-textdraw/rw-parser-symbolic-aliases.json')))]
if (directory/'closure_textdraw_sprite_symbolic.obj').exists():
 work.append(('closure_textdraw_sprite_symbolic',dict(aliases={'_r5_textdraw_ftol2_assembly_only':dict(real_symbol='__ftol2',rva='0xc7444')},functions=['?DrawSprite@R5TextDrawRenderView@@QAEXXZ'])))
if '_r5_EntitySetMatrix_thiscall_assembly_only' in maps:
 work.append(('closure_world',dict(aliases={'_r5_EntitySetMatrix_thiscall_assembly_only':dict(real_symbol='?SetMatrix@CEntity@@QAEXU_MATRIX4X4@@@Z',rva='0x9ebc0'),'_r5_EntityUpdateRw_thiscall_assembly_only':dict(real_symbol='?FUNC_1009EC80@CEntity@@QAEXXZ',rva='0x9ec80')},functions=['?SetMatrixAndUpdate@CEntity@@QAEXU_MATRIX4X4@@@Z'])))
for unit,manifest in work:
 obj=COFF(directory/(unit+'.obj'))
 for alias,item in manifest['aliases'].items():
  canonical=item['real_symbol'];address=symbol_address(maps,alias)
  assert address==symbol_address(maps,canonical) and maps[alias]==maps[canonical]
  if canonical=='__ftol2':assert maps[canonical][0][1].upper()=='LIBCMT:FTOL2.OBJ'
  if alias.startswith('_r5_Entity'):assert maps[canonical][0][1].lower()=='closure_world.obj'
  rows.append(dict(alias=alias,canonical=canonical,actual_va=address,owner=maps[alias][0][1],reference_rva=item['rva'],wrapper_bytes=0))
 for symbol in manifest['functions']:
  raw,fixes=obj.function(symbol);base=symbol_address(maps,symbol)
  for fix in fixes:
   alias=fix['symbol']['name']
   if alias not in manifest['aliases']:continue
   assert fix['kind']==20 and raw[fix['offset']-1]==0xe8
   site=base+fix['offset'];actual=(u32(pe.read(site-pe.base,4),0)+site+4)&0xffffffff
   assert actual==symbol_address(maps,manifest['aliases'][alias]['real_symbol'])
   calls.append(dict(function=symbol,offset=fix['offset'],alias=alias,actual_target=actual))
result=dict(run=run,aliases=rows,calls=calls)
Path('build/'+run+'-aliases.json').write_text(json.dumps(result,indent=2)+'\n')
print(json.dumps(result))
