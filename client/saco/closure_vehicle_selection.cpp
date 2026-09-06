#include "main.h"
#include "game/keystuff.h"
#include <stdio.h>
extern CGame *pGame;
extern int iGameDebugType;
extern DWORD dwDebugEntity1,dwDebugEntity2;
extern BOOL r5SelectionInitialized;
extern CVehicle *r5SelectionVehicle;
extern int r5SelectionModel;
extern GTA_CONTROLSET *r5SelectionControls;
extern CCamera *r5SelectionCamera;
class R5GameVehicleCreationView {
 BYTE prefix[0x6e];
 BYTE keepLoadedVehicles[212];
public:
 CVehicle *NewVehicle(int model,float x,float y,float z,float rotation,int creationFlag);
};
void GameBuildRecreateVehicle()
{
	if(r5SelectionVehicle) delete r5SelectionVehicle;

	CHAR blank[2] = "";
	r5SelectionVehicle = ((R5GameVehicleCreationView*)pGame)->NewVehicle(r5SelectionModel,5.0f,5.0f,500.0f,0.0f,0);
}
void GameBuildSelectVehicle()
{
	if(!r5SelectionInitialized) {
		r5SelectionControls = GameGetInternalKeys();
		r5SelectionCamera = pGame->GetCamera();
		r5SelectionCamera->SetPosition(-4.0f,-4.0f,502.0f,0.0f,0.0f,0.0f);
		r5SelectionCamera->LookAtPoint(5.0f,5.0f,500.0f,1);
		pGame->FindPlayerPed()->TogglePlayerControllable(0);
		pGame->DisplayGameText("Vehicle Select",4000,6);
		GameBuildRecreateVehicle();
		r5SelectionInitialized = TRUE;
	}

	pGame->DisplayHud(FALSE);

	if(r5SelectionVehicle && r5SelectionVehicle->m_pEntity) {
		VECTOR vecTurn = { 0.0f, 0.0f, 0.03f };
		VECTOR vecMove = { 0.0f, 0.0f, 0.0f };
		r5SelectionVehicle->SetTurnSpeedVector(vecTurn);
		r5SelectionVehicle->SetMoveSpeedVector(vecMove);

		MATRIX4X4 mat;
		r5SelectionVehicle->GetMatrix(&mat);
		mat.pos.X = 5.0f;
		mat.pos.Y = 5.0f;
		mat.pos.Z = 500.0f;
		r5SelectionVehicle->SetMatrix(mat);
	}

	if(r5SelectionControls->wKeys1[14] && !r5SelectionControls->wKeys2[14]) {
		r5SelectionModel--;
		if(r5SelectionModel==538) r5SelectionModel-=2; // trains
		if(r5SelectionModel==399) r5SelectionModel=611;
		GameBuildRecreateVehicle();
		return;
	}
	
	if(r5SelectionControls->wKeys1[16] && !r5SelectionControls->wKeys2[16]) {
		r5SelectionModel++;
		if(r5SelectionModel==537) r5SelectionModel+=2;  // trains
		if(r5SelectionModel==612) r5SelectionModel=400;
		GameBuildRecreateVehicle();
		return;
	}
		
	if(r5SelectionControls->wKeys1[15] && !r5SelectionControls->wKeys2[15]) {

		delete r5SelectionVehicle;
		r5SelectionVehicle = NULL;

		r5SelectionCamera->SetBehindPlayer();
		iGameDebugType=0; dwDebugEntity1=0; dwDebugEntity2=0;

		// place this vehicle near the player.
		CPlayerPed *pPlayer = pGame->FindPlayerPed();

		if(pPlayer) 
		{
			MATRIX4X4 matPlayer;
			pPlayer->GetMatrix(&matPlayer);
			CHAR blank[9] = "";
			sprintf(blank, "TYPE_%d", r5SelectionModel);
			CVehicle *pTestVehicle = ((R5GameVehicleCreationView*)pGame)->NewVehicle(r5SelectionModel,
				(matPlayer.pos.X - 5.0f), (matPlayer.pos.Y - 5.0f),
				matPlayer.pos.Z+1.0f,0.0f,0);
			
			if(pTestVehicle) {
				pPlayer->PutDirectlyInVehicle(pTestVehicle->m_dwGTAId,0);
			}		
		}

		r5SelectionCamera->Restore();
		r5SelectionCamera->SetBehindPlayer();	
		pGame->FindPlayerPed()->TogglePlayerControllable(1);
		pGame->DisplayHud(TRUE);
		r5SelectionInitialized=FALSE;

		return;
	}
}
void GameDebugDrawDebugScreens()
{
	if(!iGameDebugType) return;

	//if(pCmdWindow->isEnabled()) return;

#ifdef _DEBUG

	if(iGameDebugType==1)
	{
		GameDebugDrawActorInfo();
		return;
	}
	
	if(iGameDebugType==2 || iGameDebugType==3)
	{
		GameDebugDrawVehicleInfo();
		return;
	}

	if(iGameDebugType==5)
	{
		GameDrawMemoryInfo();
		return;
	}

	if(iGameDebugType==15)
	{
		GameDrawMemoryInfoAscii();
		return;
	}

	if(iGameDebugType==6)
	{
		GameDrawDebugTextInfo();
		return;
	}

	if(iGameDebugType==7)
	{
		GameDoVehicleSyncTest();
		return;
	}

	if (iGameDebugType==8)
	{
		GameDebugDrawTaskInfo();
		return;
	}

#endif //_DEBUG

	if(iGameDebugType==10)
	{
		GameBuildSelectVehicle();
		return;
	}
}
