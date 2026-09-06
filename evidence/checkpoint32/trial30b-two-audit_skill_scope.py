from pathlib import Path
import json,hashlib
b=Path('build/agent-network-b');m=json.load(open(b/'skill-reviewed-manifest.json'));p=json.load(open('evidence/checkpoint32/trial30-final-acceptance.json'));c=json.load(open('config/checkpoint32/trial30-final-contract.json'));by={r['anchor']:r for r in p['regions']};out={}
for n,v in m['external_targets'].items():
 if n in c['externals']:assert c['externals'][n]['reference_va']==v;out[n]=c['externals'][n]
 elif n in by and by[n]['rva']+0x10000000==v:out[n]={k:by[n][k] for k in ('unit','rva','size')}
 elif any(r['rva']+0x10000000<=v<r['rva']+0x10000000+r['size'] for r in m['regions']):out[n]={'whole_local_region_verified':True,'reference_va':v}
 else:raise Exception((n,hex(v)))
scope=dict(status='PASS_WHOLE_R5_SKILL_RPC_AND_EXISTING_REAL_VIEWS',new_unique_code=330,source025_direct_bytes=0,providers=out,scope=[
'Original RPC34 atF5E0 is the SetWeaponSkillLevel handler: reads16bit playerID,32bit skill,16bit level using actual BitStream and branches on actual local ID.',
'Local path uses accepted real CGame::FindPlayerPed, null check and CPlayerPed::SetWeaponSkillLevel(int,WORD).',
'Remote path checks original1004 slot bound/state, actual CNetPlayer pointer then remote pointer10, accepted IsActive helper and actual ped1DD. Token-identical declaration of existing R5LabelRemotePlayerView verified separately.',
'Pool view only reads existing memory, never allocated; actual CPlayerPool inline GetAt/GetLocalPlayerID retained. No new storage, complete allocated types unchanged.',
'Entire original309byte function plus21byteEH newly qualified. Actual old FindPlayerPed94+EH21 duplicates fully compared and excluded from gain. All6 whole regions31fixups and PE relocation sets exact.',
'Original model/state/invalid pointer behavior preserved; no extra validation.',
'No025 direct source found for this RPC, reconstructed from R5 and existing true providers. No callback implementation credit.'
],snapshots=[dict(path=str(f),sha256=hashlib.sha256(f.read_bytes()).hexdigest()) for f in [Path('client/saco/closure_rpc_player_skill.cpp'),Path('client/saco/closure_label_pool_helpers.cpp'),Path('client/saco/net/playerpool.h'),Path('client/saco/net/netplayer.h')]])
(b/'skill-scope-reviewed.json').write_text(json.dumps(scope,indent=2)+'\n');print(len(out))
