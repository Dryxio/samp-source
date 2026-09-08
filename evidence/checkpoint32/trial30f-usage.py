import json,sys
from pathlib import Path
start=sys.argv[1] if len(sys.argv)>1 else '2026-09-08T14:53:42Z'
end=sys.argv[2] if len(sys.argv)>2 else '9999'
root=Path('/Users/salimtrouve/.codex/sessions/2026/09/05/rollout-2026-09-05T01-58-43-01a06f13-2f75-7801-895c-a32aa0077641.jsonl')
files={'root':root}
for p in Path('/Users/salimtrouve/.codex/sessions/2026/09/08').glob('*.jsonl'):
 
 meta=json.loads(p.open().readline())['payload'];source=meta.get('source',{});a=source.get('subagent',{}).get('thread_spawn',{}).get('agent_path','') if isinstance(source,dict) else ''
 if a.endswith('_trial_'+(sys.argv[3] if len(sys.argv)>3 else 'f')):files[a]=p
out={}
for name,p in files.items():
 before={};latest={};ctx={};stamp=None
 for line in p.open():
  if '"token_count"' not in line and '"turn_context"' not in line:continue
  d=json.loads(line);t=d.get('timestamp','')
  if t>end:continue
  if d['type']=='turn_context':
   ctx={k:d['payload'].get(k) for k in ['model','effort']}
   continue
  if d['type']!='event_msg' or d['payload'].get('type')!='token_count':continue
  info=d['payload'].get('info')
  if not info:continue
  total=info['total_token_usage']
  if t<start:before=total
  else:latest=total;stamp=t
 delta={k:v-before.get(k,0) for k,v in latest.items()};delta['uncached_input_tokens']=delta.get('input_tokens',0)-delta.get('cached_input_tokens',0)
 out[name]=dict(configuration=ctx,usage=delta,as_of=stamp)
print(json.dumps(out,indent=2))
