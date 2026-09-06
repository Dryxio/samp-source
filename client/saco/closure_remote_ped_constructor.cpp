// R5 B0CE0/694. Requires real six-argument CPlayerPed overload declaration.
// Source0.2.5 constructor adapted to R5 existing full CPlayerPed layout/lifetime.
#include "main.h"
#include "game/util.h"
#include "game/keystuff.h"
#include "game/scripting.h"
extern CNetGame *pNetGame;
extern const SCRIPT_COMMAND r5CreatePreviewPlayer;
extern const SCRIPT_COMMAND r5CreatePreviewActorFromPlayer;
class R5PreviewPedModelView {
    BYTE unknown0[0x44]; DWORD gtaId; BYTE unknown48[0x25C]; PED_TYPE *ped;
public: void SetModelIndex(UINT model);
};
struct R5PedConstructorSettingsView { BYTE unknown0; int deathDropMoney; };
struct R5PedConstructorNetView { BYTE unknown0[0x3D5]; R5PedConstructorSettingsView *settings; };
CPlayerPed::CPlayerPed(BYTE playerNumber,int skin,float x,float y,float z,float rotation)
{
    DWORD actorId=0;
    int playerSlot=playerNumber;
    m_pPed=0;
    m_dwGTAId=0;
    ScriptCommand(&r5CreatePreviewPlayer,&playerSlot,x,y,z,&actorId);
    ScriptCommand(&r5CreatePreviewActorFromPlayer,&playerSlot,&actorId);
    m_dwGTAId=actorId;
    m_pPed=GamePool_Ped_GetAt(m_dwGTAId);
    m_pEntity=(ENTITY_TYPE*)m_pPed;
    m_bytePlayerNumber=playerNumber;
    SetPlayerPedPtrRecord(m_bytePlayerNumber,(DWORD)m_pPed);
    ScriptCommand(&set_actor_weapon_droppable,m_dwGTAId,1);
    ScriptCommand(&set_actor_immunities,m_dwGTAId,0,0,1,0,0);
    ScriptCommand(&set_actor_can_be_decapitated,m_dwGTAId,0);
    if(pNetGame) {
        int money=((R5PedConstructorNetView*)pNetGame)->settings->deathDropMoney;
        ScriptCommand(&set_actor_money,m_dwGTAId,0);
        ScriptCommand(&set_actor_money,m_dwGTAId,money);
    }
    field_2A8=1;
    field_2AC=1;
    m_dwArrow=0;
    field_2B9=0;
    m_iCellPhoneEnabled=0;
    m_bGoggleState=FALSE;
    m_iDanceState=0;
    field_2F2=0;
    field_2DE=0;
    field_2E2=0;
    ((R5PreviewPedModelView*)this)->SetModelIndex(skin);
    // Actual R5 inline SetTargetRotation semantics; do not duplicate its owner.
    if(m_pPed && GamePool_Ped_GetAt(m_dwGTAId)) {
        m_pPed->fRotation2=DegToRad(rotation);
        m_pPed->fRotation1=DegToRad(rotation);
    }
    GameResetPlayerKeys(m_bytePlayerNumber);
    field_2C1=0;
    field_2C5=0;
    field_2C9=0;
    field_2F6=0;
    field_2F7=0;
    field_2F8=0;
    *(DWORD*)(_gap2FD+44)=0;
    memset(_gap2FD,0,44);
    MATRIX4X4 matrix;
    GetMatrix(&matrix);
    matrix.pos.X=x;
    matrix.pos.Y=y;
    matrix.pos.Z=z+0.15f;
    SetMatrix(matrix);
    int index=0;
    while(index!=10) {
        field_4C[index]=0;
        field_27C[index]=0;
        memset(&field_74[index],0,sizeof(struc_97));
        index++;
    }
    field_2F9=0;
    *(DWORD*)(_gap2FD+44)=0;
    memset(_gap2FD,0,44);
}
