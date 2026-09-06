#include "main.h"
extern CCmdWindow *pCmdWindow;
extern GAME_SETTINGS tSettings;
void cmdDefaultCmdProc(PCHAR);
void cmdQuit(PCHAR);
void cmdSavePos(PCHAR);
void cmdRawSavePos(PCHAR);
void cmdRcon(PCHAR);
void cmdMem(PCHAR);
void cmdSetFrameLimit(PCHAR);
void cmdPageSize(PCHAR);
void cmdFontSize(PCHAR);
void cmdNameTagStatus(PCHAR);
void cmdTimestamp(PCHAR);
void cmdHeadMove(PCHAR);
void cmdHudScaleFix(PCHAR);
void cmdTestDeathWindow(PCHAR);
void cmdSelectVehicle(PCHAR);
void cmdCreateVehicle(PCHAR);
void cmdPlayerSkin(PCHAR);
void cmdSetWeather(PCHAR);
void cmdSetTime(PCHAR);
void cmdShowInterior(PCHAR);
void cmdToggleObjectLight(PCHAR);
void cmdCmpStat(PCHAR);
void cmdDebugLabels(PCHAR);
void cmdCameraTargetDebug(PCHAR);
void cmdAudioMsg(PCHAR);
void cmdLogUrls(PCHAR);
void SetupCommands()
{
	// RELEASE COMMANDS
	pCmdWindow->AddDefaultCmdProc(cmdDefaultCmdProc);
	pCmdWindow->AddCmdProc("quit",cmdQuit);
	pCmdWindow->AddCmdProc("q",cmdQuit);
	pCmdWindow->AddCmdProc("save",cmdSavePos);
	pCmdWindow->AddCmdProc("rs",cmdRawSavePos);
	pCmdWindow->AddCmdProc("rcon",cmdRcon);
	pCmdWindow->AddCmdProc("mem",cmdMem);
	pCmdWindow->AddCmdProc("fpslimit",cmdSetFrameLimit);
	pCmdWindow->AddCmdProc("pagesize",cmdPageSize);
	pCmdWindow->AddCmdProc("fontsize",cmdFontSize);
	pCmdWindow->AddCmdProc("nametagstatus",cmdNameTagStatus);
	pCmdWindow->AddCmdProc("timestamp",cmdTimestamp);
	pCmdWindow->AddCmdProc("headmove",cmdHeadMove);
	pCmdWindow->AddCmdProc("hudscalefix",cmdHudScaleFix);
	pCmdWindow->AddCmdProc("testdw",cmdTestDeathWindow);

#ifndef _DEBUG
	if(tSettings.bDebug)
	{
#endif
	pCmdWindow->AddCmdProc("vsel",cmdSelectVehicle);
	pCmdWindow->AddCmdProc("v",cmdCreateVehicle);
	pCmdWindow->AddCmdProc("vehicle",cmdCreateVehicle);
	pCmdWindow->AddCmdProc("player_skin",cmdPlayerSkin);
	pCmdWindow->AddCmdProc("set_weather",cmdSetWeather);
	pCmdWindow->AddCmdProc("set_time",cmdSetTime);
#ifndef _DEBUG
	}
#endif

	pCmdWindow->AddCmdProc("interior",cmdShowInterior);
	pCmdWindow->AddCmdProc("togobjlight",cmdToggleObjectLight);
	pCmdWindow->AddCmdProc("cmpstat",cmdCmpStat);
	pCmdWindow->AddCmdProc("dl",cmdDebugLabels);
	pCmdWindow->AddCmdProc("ctd",cmdCameraTargetDebug);
	pCmdWindow->AddCmdProc("audiomsg",cmdAudioMsg);
	pCmdWindow->AddCmdProc("logurls",cmdLogUrls);
}
