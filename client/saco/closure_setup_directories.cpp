#include "main.h"
#include "game/util.h"
#include <stdio.h>
#include <string.h>
extern CHAR szSAMPDir[MAX_PATH+1];
extern CHAR szCacheDir[MAX_PATH+1];

void SetupCacheDirectories()
{
	ZeroMemory(szCacheDir, sizeof(szCacheDir));

	CHAR szPath[MAX_PATH+1];
	DWORD cbData = MAX_PATH;
	HKEY hKey;
	DWORD dwType;
	ZeroMemory(szPath, sizeof(szPath));

	sprintf(szCacheDir, "%s\\cache", szSAMPDir);
	if(RegOpenKeyEx(HKEY_CURRENT_USER, "Software\\SAMP", 0, KEY_READ, &hKey) == ERROR_SUCCESS &&
		RegQueryValueEx(hKey, "model_cache", NULL, &dwType, (LPBYTE) szPath, &cbData) == ERROR_SUCCESS)
	{
		strncpy(szCacheDir, szPath, MAX_PATH+1);
	}
	if(!IsFileOrDirectoryExists(szCacheDir))
		CreateDirectoryA(szCacheDir, NULL);

	CHAR szLocalDir[MAX_PATH+1];
	sprintf(szLocalDir, "%s\\local", szCacheDir);
	if(!IsFileOrDirectoryExists(szLocalDir))
		CreateDirectoryA(szLocalDir, NULL);
}

void SetupDirectories()
{
	ZeroMemory(szSAMPDir, sizeof(szSAMPDir));

	if(strlen((char*)0xC92368) == 0) {
		GetCurrentDirectory(MAX_PATH+1, szSAMPDir);
	} else {
		sprintf(szSAMPDir, "%s\\SAMP", (char*)0xC92368);
		if(!IsFileOrDirectoryExists(szSAMPDir))
			CreateDirectoryA(szSAMPDir, NULL);

		CHAR szScreensDir[MAX_PATH+1];
		sprintf(szScreensDir, "%s\\screens", szSAMPDir);
		if(!IsFileOrDirectoryExists(szScreensDir))
			CreateDirectoryA(szScreensDir, NULL);

		SetupCacheDirectories();
	}
}

