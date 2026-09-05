"""Prepare complete source definitions needed for the ActorPed link closure."""
import json
from pathlib import Path
from verify import ROOT
from prepare_checkpoint32 import definition

UNITS={
 'closure_world':('game/entity.cpp',['GetModelIndex','GetMatrix','SetMoveSpeedVector','SetTurnSpeedVector','Add','Remove','TeleportTo','FUNC_1009EC80','SetMatrix','GetMoveSpeedVector','GetTurnSpeedVector','IsAdded','ApplyTurnSpeed','GetDistanceFromCentreOfMassToBaseOfModel','GetBoundCentre','GetBoundRect','GetDistanceFromCamera','GetDistanceFromPoint','EnforceWorldBoundries','HasExceededWorldBoundries','SetCollisionChecking','IsCollisionCheckingEnabled','SetGravityProcessing','SetWaitingForCollision','DisableStreaming','EnableTunnelTransition','SetApplySpeed','MakeNonCollidable','SetClumpAlpha','GetWorldBoundRadius','SetEulerAngles','GetEulerAngles','ApplyForce','PlayAudio','GetRwObject','IsNativeTarget','DeleteRwObject','ProcessControl','SetMatrixAndUpdate','AdvancePosition','IsStationary'],'CEntity'),
 'closure_chat':('chatwindow.cpp',['PushBack','AddDebugMessage','AddEntry','Log'],'CChatWindow'),
 'closure_util':('game/util.cpp',['WorldAddEntity','WorldRemoveEntity','GamePool_Ped_GetAt','DegToRad'],None),
 'closure_filter':('exceptions.cpp',['exc_filter'],None),
 'closure_models':('game/game.cpp',['RequestModel','LoadRequestedModels','IsModelLoaded','SetWorldTime','RequestAnimation','IsAnimationLoaded'],'CGame')}


def prepare():
 units=dict(UNITS);records={}
 selection=ROOT/'config/checkpoint32/common-selection.json'
 if selection.exists():
  records=json.loads(selection.read_text())
  for unit,record in records.items():
   base=units.get(unit,(record['source'],[],record['cls']))
   units[unit]=(base[0],list(base[1])+record['definitions'],base[2])
 for unit,(file,names,cls) in units.items():
  s=(ROOT/'client/saco'/file).read_bytes().decode('latin1')
  header='// Generated complete definitions from '+file+'; see tools/prepare_actor_closure.py.\n#include <time.h>\n#include <math.h>\n#include "main.h"\n#include "game/util.h"\n'
  if unit=='closure_models':header+='// Original game.cpp draw-zone callback declaration.\ntypedef void (*DrawZone_t)(float *fPos, DWORD *dwColor, BYTE byteMenu);\n'
  if unit=='closure_filter':header+='extern CChatWindow *pChatWindow;\nextern DWORD dwScmOpcodeDebug;\nextern WORD wVehicleComponentDebug;\nint dword_10125A58=0;\n'
  if unit=='closure_util':header+='#include <sys/stat.h>\n#undef PI\n#define PI 3.14159265f\n'
  record=records.get(unit,{})
  header+='\n'.join(record.get('declarations',[]))+'\n' if record.get('declarations') else ''
  if 'storage' in record:
   storage=record['storage']
   if storage['source_declaration'] not in s:raise ValueError('missing original storage declaration '+unit)
   header+=storage['declaration']+'\n'
   header+='typedef char complete_storage_size[(sizeof('+storage['symbol']+')=='+str(storage['size'])+')?1:-1];\n'
  code=header+'\n\n'+'\n\n'.join(definition(s,(cls+'::' if cls else '')+(n['name'] if isinstance(n,dict) else n),n.get('overload') if isinstance(n,dict) else None) for n in names)+'\n'
  (ROOT/'client/saco'/(unit+'.cpp')).write_bytes(code.encode('latin1'))
 # These are real zero-initialized client globals, copied from main.cpp.
 (ROOT/'client/saco/closure_state.cpp').write_text('#include "main.h"\nCChatWindow *pChatWindow=0;\nWORD wVehicleComponentDebug=0;\nCGame *pGame=0;\n')

if __name__=='__main__':prepare()
