import sys,subprocess
from pathlib import Path
run=sys.argv[1];prefix=sys.argv[2]
commands=[['build/prove_linker_aliases.py',run]]
for n in ['itoa','sha1_alias']:
 s=Path('build/qualify_trial30_final_'+n+'.py').read_text().replace('cp32-trial30-final-linked1',run).replace('trial30-final-',prefix+'-')
 f='build/qualify_'+prefix.replace('-','_')+'_'+n+'.py';Path(f).write_text(s);commands.append([f])
for f in ['build/agent-independent/qualify_server_bridge_target_alias.py','build/agent-independent/qualify_logger_deleting_aliases.py','build/agent-independent/qualify_replica_network_id_aliases.py','build/qualify_mesh_deleting_alias.py','build/agent-independent/qualify_console_aliases.py','build/agent-independent/qualify_raknet_command_parser_alias.py','build/agent-textdraw/qualify_chat_navigation_alias.py']:commands.append([f,'--run',run])
for args in commands:subprocess.run([sys.executable,*args],check=True)
