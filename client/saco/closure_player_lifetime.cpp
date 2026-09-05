// Proposed lifetime reconstruction. Requires the separate header delta.
#include "main.h"
#include "game/util.h"
#include "game/keystuff.h"
#include <stddef.h>
// Compile-time ABI assertions under the existing /Zp1 x86 build, no runtime tests.
typedef char RPCPedSize[(sizeof(CPlayerPed)==0x32d)?1:-1];
typedef char RPCEntitySize[(sizeof(CEntity)==0x48)?1:-1];
typedef char RPCPedSlotsOffset[(offsetof(CPlayerPed,field_4C)==0x4c)?1:-1];
typedef char RPCPedAttachmentDataOffset[(offsetof(CPlayerPed,field_74)==0x74)?1:-1];
typedef char RPCPedObjectsOffset[(offsetof(CPlayerPed,field_27C)==0x27c)?1:-1];
typedef char RPCPedNativeOffset[(offsetof(CPlayerPed,m_pPed)==0x2a4)?1:-1];
typedef char RPCPedNumberOffset[(offsetof(CPlayerPed,m_bytePlayerNumber)==0x2b0)?1:-1];
typedef char RPCPedSlotSize[(sizeof(struc_97)==0x34)?1:-1];
const SCRIPT_COMMAND rpc_ped_disassociate_object={0x0682,"ifffi"};
const SCRIPT_COMMAND rpc_ped_destroy_object_with_fade={0x09a2,"i"};
const SCRIPT_COMMAND rpc_ped_carry_object={0x070a,"iifffiissi"};
CPlayerPed::CPlayerPed()
{
	m_dwGTAId = 1; // 0x001
	m_pPed = GamePool_FindPlayerPed();
	m_pEntity = (ENTITY_TYPE *)GamePool_FindPlayerPed();

	m_bytePlayerNumber = 0;
	SetPlayerPedPtrRecord(m_bytePlayerNumber,(DWORD)m_pPed);
	ScriptCommand(&set_actor_weapon_droppable,m_dwGTAId,1);
	ScriptCommand(&set_actor_can_be_decapitated,m_dwGTAId,0);

	field_2A8 = 1;
	field_2AC = 1;
	m_dwArrow = 0;
	field_2B9 = 0;
	field_2F2 = 0;
	m_iDanceState = 0;
	field_2DE = 0;
	field_2E2 = 0;
	m_iCellPhoneEnabled = 0;
	m_bGoggleState = FALSE;
	field_2C1 = 0;
	field_2C5 = 0;
	field_2C9 = 0;
	field_2F6 = 0;
	field_2F7 = 0;
	field_2F8 = 0;

	int x=0;
	while(x!=10) {
		field_4C[x] = 0;
		field_27C[x] = 0;
		memset(&field_74[x], 0, sizeof(struc_97));
		x++;
	}

	field_2F9 = 0;
}


CPlayerPed::~CPlayerPed() { Destroy(); }
int CPlayerPed::HasObjectAttached() {
 for(int i=0;i<10;i++) if(field_4C[i]==1)return 1;
 return 0;
}
void CPlayerPed::StopCarrying() {
 if(field_2C5) {
  ScriptCommand(&rpc_ped_carry_object,m_dwGTAId,field_2C5,0.0f,0.0f,0.0f,6,16,"NULL","NULL",0);
  field_2C5=0;
 }
 MATRIX4X4 mat;
 GetMatrix(&mat);
 TeleportTo(mat.pos.X,mat.pos.Y,mat.pos.Z);
 field_2C1=0;
}
void CPlayerPed::Destroy() {
 DWORD dwPedPtr=(DWORD)m_pPed;
 GameResetPlayerKeys(m_bytePlayerNumber);
 SetPlayerPedPtrRecord(m_bytePlayerNumber,0UL);
 if(!m_pPed || !GamePool_Ped_GetAt(m_dwGTAId) || m_pPed->entity.vtable==0x863c40) {
  m_pPed=NULL; m_pEntity=NULL; m_dwGTAId=0; return;
 }
 if(field_2B9) {
  ScriptCommand(&rpc_ped_disassociate_object,field_2B9,0.0f,0.0f,0.0f,0);
  ScriptCommand(&rpc_ped_destroy_object_with_fade,field_2B9);
  field_2B9=0;
 }
 if(HasObjectAttached())RemoveAllAttachedObjects();
 if(field_2C1)StopCarrying();
 if(IN_VEHICLE(m_pPed))RemoveFromVehicleAndPutAt(100.0f,100.0f,10.0f);
 // Explicit symbolic assembly: original 0.2.5 native destruction protocol,
 // also observed in R5. No instruction bytes are emitted or copied.
 _asm mov esi,dwPedPtr
 _asm mov eax,[esi+1152]
 _asm mov dword ptr[eax+76],0
 _asm mov ecx,dwPedPtr
 _asm mov ebx,[ecx]
 _asm push 1
 _asm call [ebx]
 m_pPed=NULL; m_pEntity=NULL;
}
// Taking the genuine inline member address forces a dedicated FindPlayerPed body.
CPlayerPed *(CGame::*rpc_find_player_ped_body)()=&CGame::FindPlayerPed;
