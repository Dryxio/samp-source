#include "main.h"
#include "game/util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
extern CGame *pGame;
extern CChatWindow *pChatWindow;
extern char *GetSAMPPath();
// Exact declaration of the existing pointer-only info path, not AddDebug.
class R5ChatInfoView {
    BYTE reserved0[0x126];
    DWORD infoColor;
public:
    void AddInfoMessage(char *format,...);
};
// Full real 18-byte script descriptors observed at E9D98 and E9DC0.
extern const SCRIPT_COMMAND r5CmdGetCarZAngle;
extern const SCRIPT_COMMAND r5CmdGetActorZAngle;

void cmdSavePos(PCHAR szCmd)
{
	CPlayerPed *pPlayer = pGame->FindPlayerPed();
	FILE *fileOut;
	DWORD dwVehicleID;
	float fZAngle;

	//if(!tSettings.bDebug) return;

	char path[MAX_PATH];
	sprintf(path,"%s\\savedpositions.txt",GetSAMPPath());
	fileOut = fopen(path,"a");
	if(!fileOut) {
		pChatWindow->AddDebugMessage("I can't open the savepositions.txt file for append.");
		return;
	}

	// incar savepos

	if(pPlayer->IsInVehicle()) {

		VEHICLE_TYPE *pVehicle = pPlayer->GetGtaVehicle();
	
		dwVehicleID = GamePool_Vehicle_GetIndex(pVehicle);
		ScriptCommand(&r5CmdGetCarZAngle,dwVehicleID,&fZAngle);

		fprintf(fileOut,"AddStaticVehicle(%u,%.4f,%.4f,%.4f,%.4f,%u,%u); // %s\n",
			pVehicle->entity.nModelIndex,pVehicle->entity.mat->pos.X,pVehicle->entity.mat->pos.Y,pVehicle->entity.mat->pos.Z,
			fZAngle,pVehicle->byteColor1,pVehicle->byteColor2,szCmd);

		fclose(fileOut);
		((R5ChatInfoView*)pChatWindow)->AddInfoMessage("-> InCar position saved");

		return;
	}

	// onfoot savepos

	PED_TYPE *pActor = pPlayer->m_pPed;
	ScriptCommand(&r5CmdGetActorZAngle,pPlayer->m_dwGTAId,&fZAngle);

	fprintf(fileOut,"AddPlayerClass(%u,%.4f,%.4f,%.4f,%.4f,0,0,0,0,0,0); // %s\n",pPlayer->GetModelIndex(),
		pActor->entity.mat->pos.X,pActor->entity.mat->pos.Y,pActor->entity.mat->pos.Z,fZAngle,szCmd);

	fclose(fileOut);
	((R5ChatInfoView*)pChatWindow)->AddInfoMessage("-> OnFoot position saved");
}

void cmdRawSavePos(PCHAR szCmd)
{
	CPlayerPed *pPlayer = pGame->FindPlayerPed();
	FILE *fileOut;
	DWORD dwVehicleID;
	float fZAngle;

	if(pPlayer->IsInVehicle()) {

		fileOut = fopen("rawvehicles.txt","a");
		if(!fileOut) {
			pChatWindow->AddDebugMessage("I can't open the rawvehicles.txt file for append.");
			return;
		}

		VEHICLE_TYPE *pVehicle = pPlayer->GetGtaVehicle();
	
		dwVehicleID = GamePool_Vehicle_GetIndex(pVehicle);
		ScriptCommand(&r5CmdGetCarZAngle,dwVehicleID,&fZAngle);

		fprintf(fileOut,"%u,%.4f,%.4f,%.4f,%.4f,%u,%u ; %s\n",
			pVehicle->entity.nModelIndex,pVehicle->entity.mat->pos.X,pVehicle->entity.mat->pos.Y,pVehicle->entity.mat->pos.Z,
			fZAngle,pVehicle->byteColor1,pVehicle->byteColor2,szCmd);

		fclose(fileOut);

        pChatWindow->AddDebugMessage("-> InCar pos saved");
		return;
	}

	// onfoot savepos

	PED_TYPE *pActor = pPlayer->m_pPed;
	ScriptCommand(&r5CmdGetActorZAngle,pPlayer->m_dwGTAId,&fZAngle);
	
	char path[MAX_PATH];
	sprintf(path,"%s\\rawpositions.txt",GetSAMPPath());
	fileOut = fopen(path,"a");

	if(!fileOut) {
		pChatWindow->AddDebugMessage("I can't open the rawvehicles.txt file for append.");
		return;
	}

	fprintf(fileOut,"%.4f,%.4f,%.4f,%.4f ; %s\n",pActor->entity.mat->pos.X,pActor->entity.mat->pos.Y,pActor->entity.mat->pos.Z,fZAngle,szCmd);
	fclose(fileOut);

	pChatWindow->AddDebugMessage("-> OnFoot pos saved");
}
