from pathlib import Path
s=Path('build/agent-textdraw/preflight_cmd_ui_complete.py').read_text()
s=s.replace("D=Path('build/cp32-policy-ui-ob1')", "D=Path('build/cp32-cmd-registration-leaves-trial1')").replace('rsa-commands-contract.json','policy-ui-contract.json').replace('rsa-commands-acceptance.json','policy-ui-acceptance.json')
a=s.index('providers.update({\'?NewVehicle');b=s.index('for u,n,r,z in plan:providers',a)
s=s[:a]+'''providers.update({'?dwStartQuitTick@@3KA':p.base+0x26eb34,'?iGameDebugType@@3HA':p.base+0x14ff50,'?dwDebugEntity1@@3KA':p.base+0x14ff54,'?dwDebugEntity2@@3KA':p.base+0x14ff58,'?r5RpcChat@@3HB':p.base+0xe58ec,'?pDeathWindow@@3PAVCDeathWindow@@A':p.base+0x26eb88})
plan=[('closure_cmd_registration_leaves','?'+n+'@@YAXPAD@Z',r,z) for n,r,z in [('cmdDefaultCmdProc',0x684e0,32),('cmdTestDeathWindow',0x68500,222),('cmdQuit',0x689e0,5),('cmdSelectVehicle',0x68eb0,24)]]
plan += [('closure_game_quit','?QuitGame@@YAXXZ',0xc3e80,54),('closure_local_player_say','?Say@CLocalPlayer@@QAEXPAD@Z',0x5a10,191),('closure_game_debug_entity','?GameDebugEntity@@YAXKKH@Z',0x9e0a0,30)]
''' +s[b:]
a=s.index("add('closure_cmd_rcon'");b=s.index('# Discover only',a)
s=s[:a]+'''add('closure_local_player_say',51,0xe0040);add('closure_local_player_say',52,0xf7808)
for u,n,r,z in [('closure_store_quit_start_tick','?dwStartQuitTick@@3KA',0x26eb34,4),('closure_store_game_debug_type','?iGameDebugType@@3HA',0x14ff50,4),('closure_store_game_debug_entity1','?dwDebugEntity1@@3KA',0x14ff54,4),('closure_store_game_debug_entity2','?dwDebugEntity2@@3KA',0x14ff58,4),('closure_store_rpc_chat','?r5RpcChat@@3HB',0xe58ec,4)]:
 objects[u]=COFF(D/(u+'.obj'));maps[u]={};a=next(x for x in objects[u].names[n] if x['section']>0);assert objects[u].sections[a['section']-1]['size']==z;add(u,a['section'],r,n)
''' +s[b:]
a=s.index('assert len(prior)==');s=s[:a]+'''assert len(prior)==381304;assert len(union-prior)==579,len(union-prior)
out=dict(status='CODE_EH_DATA_PASS_REQUIRED_DEATH_STORE_PENDING',run=D.name,baseline=len(prior),new_unique_code=579,principal_code=558,new_EH=21,source025_direct=54,regions=list(rows.values()),required_store=dict(unit='closure_store_death_window',anchor='?pDeathWindow@@3PAVCDeathWindow@@A',rva=0x26eb88,size=4,initial=0,accepted=False),excluded=['Unused inline helpers not credited.','SetupCommands593 and remaining font dependency not included.'],source_files={str(Path('client/saco')/(u+'.cpp')):hashlib.sha256((D/'client/saco'/(u+'.cpp')).read_bytes()).hexdigest() for u in objects})
from prepare_checkpoint32 import definition
ref=Path('/Users/salimtrouve/Documents/GitHub/samp-r5-preservation/references/michael-fa-samp/client')
for u,n,f in [('closure_cmd_registration_leaves','cmdSelectVehicle','cmdprocs.cpp'),('closure_game_debug_entity','GameDebugEntity','game/debug.cpp')]:
 actual=definition((D/'client/saco'/(u+'.cpp')).read_text(),n);reference=definition((ref/f).read_text(),n);assert actual==reference
(OUT/'cmd-registration-reviewed-manifest.json').write_text(json.dumps(out,indent=2)+'\\n');(OUT/'cmd-registration-seeds.json').write_text(json.dumps(seeds,indent=2)+'\\n');print(out['status'],len(rows),'sections',sum(len(r['bindings']) for r in rows.values()),'bindings',out['new_unique_code'],'unique')
'''
Path('build/agent-textdraw/preflight_cmd_registration.py').write_text(s)
