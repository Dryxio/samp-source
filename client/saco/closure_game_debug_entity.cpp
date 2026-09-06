#include <windows.h>
extern int iGameDebugType;
extern DWORD dwDebugEntity1,dwDebugEntity2;
void GameDebugEntity(DWORD dwEnt1, DWORD dwEnt2, int type)
{
	iGameDebugType=type;
	dwDebugEntity1=dwEnt1;
	dwDebugEntity2=dwEnt2;
}
