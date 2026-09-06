from pathlib import Path
import re,json
s=Path('vendor/upstream/raknet/SocketLayer.cpp').read_text()
pat=r'''//[^\n]*|/\*.*?\*/|"(?:\\.|[^"\\])*"|'(?:\\.|[^'\\])*' '''.rstrip()
mask=re.sub(pat,lambda m:re.sub(r'[^\n]',' ',m.group()),s,flags=re.S)
parts=[]
for name in ['Connect','CreateBoundSocket','DomainNameToIP','SendTo','GetLocalPort']:
 m=re.search(r'^.*SocketLayer::'+name+r'\s*\(',mask,re.M);b=mask.index('{',m.end());e=b+1;depth=1
 while depth:depth+=(mask[e]=='{')-(mask[e]=='}');e+=1
 parts.append(s[m.start():e])
out='#include "../raknet/SocketLayer.h"\n#include <assert.h>\n#include <string.h>\ntypedef int socklen_t;\n\n'+'\n\n'.join(parts)+'\n'
Path('client/saco/closure_vendor_socket_leaves.cpp').write_text(out)
rows=[r for r in json.load(open('evidence/checkpoint31/coverage.json'))['function_rows'] if r.get('object')=='raknet_SocketLayer.obj' and r.get('rva') and any(r['symbol'].startswith('?'+n+'@') for n in ['Connect','CreateBoundSocket','DomainNameToIP','SendTo','GetLocalPort'])]
Path('build/socket-leaves-cp31-identities.json').write_text(json.dumps(rows,indent=2)+'\n');print(sum(r['bytes'] for r in rows))
