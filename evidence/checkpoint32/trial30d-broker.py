import subprocess,json,time,threading,pathlib
D=pathlib.Path(__file__).resolve().parent
p=subprocess.Popen(['/Users/salimtrouve/.nvm/versions/node/v22.22.0/bin/codex','-c','model="gpt-5.6-luna"','-c','model_reasoning_effort="high"','-c','service_tier="fast"','--enable','fast_mode','app-server','--listen','stdio://'],stdin=subprocess.PIPE,stdout=subprocess.PIPE,stderr=open(D/'stderr.log','w'),text=True,bufsize=1)
(D/'pid.json').write_text(json.dumps({'broker':__import__('os').getpid(),'server':p.pid}))
responses={}; lock=threading.Lock()
def reader():
 with open(D/'events.jsonl','a',buffering=1) as f:
  for line in p.stdout:
   try:e=json.loads(line)
   except:continue
   f.write(json.dumps({'time':time.time(),'event':e})+'\n')
   if 'id' in e and ('result'in e or 'error'in e):responses[str(e['id'])]=e
threading.Thread(target=reader,daemon=True).start()
def send(v):
 p.stdin.write(json.dumps(v)+'\n');p.stdin.flush()
send({'id':'init','method':'initialize','params':{'clientInfo':{'name':'samp_benchmark','version':'1.0'},'capabilities':{'experimentalApi':True}}})
while 'init' not in responses:time.sleep(.05)
send({'method':'initialized','params':{}})
(D/'ready').write_text(json.dumps(responses['init']))
seen=set()
while p.poll() is None:
 for q in sorted(D.glob('cmd-*.json')):
  if q.name in seen:continue
  try:v=json.loads(q.read_text())
  except:continue
  seen.add(q.name);send(v)
 for k,v in list(responses.items()):
  out=D/('response-'+k+'.json')
  if not out.exists():out.write_text(json.dumps(v,indent=2))
 time.sleep(.1)
