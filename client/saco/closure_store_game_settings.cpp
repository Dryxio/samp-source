#include <windows.h>
#define MAX_SETTINGS_STRING 256
typedef struct _GAME_SETTINGS {
	BOOL bDebug;
	BOOL bPlayOnline;
	BOOL bWindowedMode;
	CHAR szConnectPass[MAX_SETTINGS_STRING+1];
	CHAR szConnectHost[MAX_SETTINGS_STRING+1];
	CHAR szConnectPort[MAX_SETTINGS_STRING+1];
	CHAR szNickName[MAX_SETTINGS_STRING+1];
	CHAR szDebugScript[MAX_SETTINGS_STRING+1];
} GAME_SETTINGS;
typedef char R5GameSettingsFullSize[sizeof(GAME_SETTINGS)==1297 ? 1 : -1];
GAME_SETTINGS tSettings;
